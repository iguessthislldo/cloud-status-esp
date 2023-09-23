const char * const wifi_ssid = "REPLACE THIS";
const char * const wifi_password = "REPLACE THIS";

const unsigned long cycle_len = 30;

void init_config() {
    statuses.add("lan (gateway)", 14)->ping_gateway(); // D5
    statuses.add("wan (8.8.8.8)", 12)->ping_address(IPAddress(8, 8, 8, 8)); // D6
    statuses.add("dns (google.com)", 13)->ping_hostname("google.com"); // D7
}
