struct Output {
    const int pin;
    bool is_high;

    Output(int pin, bool init_is_high)
    : pin(pin)
    , is_high(init_is_high)
    {
    }

    void init() {
        pinMode(pin, OUTPUT);
        set(is_high);
    }

    bool set(bool is_high) {
        this->is_high = is_high;
        digitalWrite(pin, is_high ? HIGH : LOW);
        return is_high;
    }

    bool toggle() {
        return set(!is_high);
    }

    void flash(unsigned long period, unsigned count = 1) {
        const auto p = period / 2;
        for (unsigned i = 0; i < count; i++) {
            set(true);
            delay(p);
            set(false);
            delay(p);
        }
    }
};

template<typename Type>
struct List {
    Type* head = nullptr;
    Type* tail = nullptr;

    void clear() {
        for (Type* value = head; value;) {
            Type* next = value->next;
            delete value;
            value = next;
        }
        head = nullptr;
        tail = nullptr;
    }

    ~List() {
        clear();
    }

    void add(Type* new_value) {
        if (!head) {
            head = new_value;
        }
        if (tail) {
            tail->next = new_value;
        }
        tail = new_value;
        new_value->next = nullptr;
    }
};

class Status {
public:
    enum Kind {
        None,
        Gateway,
        Address,
        Hostname,
    };

    const char* const name;
    Output output;
    bool success;
    Kind kind;
    unsigned reqs;
    IPAddress address;
    const char* hostname;
    Status* next = nullptr;

    Status(const char* name, int pin, unsigned reqs)
    : name(name)
    , output(pin, false)
    , success(false)
    , kind(Status::None)
    , reqs(reqs)
    {
    }

    void ping_gateway() {
        kind = Gateway;
    }

    void ping_address(const IPAddress& address) {
        kind = Address;
        this->address = address;
    }

    void ping_hostname(const char* hostname) {
        kind = Hostname;
        this->hostname = hostname;
    }

    void init() {
        output.init();
    }
};

class Statuses {
public:
    List<Status> list;

    Status* add(const char* name, int pin, unsigned reqs = 5) {
        Status* s = new Status(name, pin, reqs);
        list.add(s);
        return s;
    }

    void init_all() {
        for (Status* s = list.head; s; s = s->next) {
            s->init();
        }
    }
} statuses;
