<?php
require_once("api/framework/APITools.inc");
require_once("api/framework/APIResponse.inc");
require_once("api/framework/APIAuth.inc");

class APIBaseModel {
    public $client;
    public $privileges;
    public $initial_data;
    public $validated_data;
    public $config;
    public $errors;
    public $methods;
    public $change_note;
    public $requires_auth;
    public $set_auth_mode;

    public function __construct() {
        global $config;
        $this->config =& $config;
        $this->methods = ["GET", "POST"];
        $this->privileges = ["page-all"];
        $this->client = null;
        $this->requires_auth = true;
        $this->set_auth_mode = null;
        $this->change_note = "Made unknown change via API";
        $this->initial_data = APITools\get_request_data();
        $this->validated_data = [];
        $this->errors = [];

    }

    public function action() {
        # This function is intended to be overridden by an API model extended class
        # Any configuration writes, system configurations, etc should be added when overriding this base class
        # If this class is not overridden a 500 unexpected error is returned
        return APIResponse\get(10);
    }

    protected function write_config() {
        $_SESSION["Username"] = $this->client->username;    // Save our user to session data for logging
        write_config(sprintf(gettext(" ".$this->change_note)));    // Apply our configuration change
        unset($_SESSION);
    }

    public function validate_payload() {
        # This function is intended to be overridden by an API model extended class
        # Any payload validation must be specified here
        # If this class is not overridden no validation takes place
    }

    public function validate() {
        $this->check_enable();
        $this->check_server_ip();
        $this->check_version();
        $this->check_method();
        if ($this->requires_auth) {
            $this->check_authentication();
            $this->check_authorization();
        }
        $this->validate_payload();

        if (count($this->errors) === 0) {
            return true;
        } else {
            return false;
        }
    }

    public function call() {
        if ($this->validate()) {
            return $this->action();
        } else {
            return $this->errors[0];
        }
    }

    public function listen() {
        header("Content-Type: application/json", true);
        header("Referer: no-referrer");
        $resp = $this->call();
        http_response_code($resp["code"]);
        echo json_encode($resp) . PHP_EOL;
        exit();
    }

    private function check_authentication() {
        $this->client = new APIAuth($this->privileges, $this->set_auth_mode);
        if ($this->requires_auth === true) {
            if (!$this->client->is_authenticated) {
                $this->errors[] = APIResponse\get(3);
            }
        }
    }

    private function check_authorization() {
        if (!$this->client->is_authorized) {
            $this->errors[] = APIResponse\get(4);
        }
    }

    private function check_method() {
        if (!in_array($_SERVER["REQUEST_METHOD"], $this->methods)) {
            $this->errors[] = APIResponse\get(2);
        }
    }

    # Check if the API is enabled before answering calls, if not, redirect to wc login
    private function check_enable() {
        $api_config = APITools\get_api_config()[1];
        if (!isset($api_config["enable"])) {
            header("Location: /");
            die();
        }
    }

    # Check if server is running a supported version of pfSense
    private function check_version() {
        # Local variables
        $curr_ver = str_replace(".", "", explode("-", APITools\get_pfsense_version()["version"])[0]);
        $min_ver = 244;
        $curr_ver = is_numeric($curr_ver) ? intval($curr_ver) : 0;
        if ($curr_ver < $min_ver) {
            $this->errors[] = APIResponse\get(5);
        }
    }

    # Check if server IP is allowed to answer API calls. Redirects to login if not
    private function check_server_ip() {
        $pkg_conf = APITools\get_api_config()[1];
        $allow_ifs = $pkg_conf["allowed_interfaces"];
        $whitelist = explode(",", $allow_ifs);

        // Check if our server IP is in our whitelist
        foreach ($whitelist as $wif) {
            $if_info = get_interface_info($wif);
            // Check if our server IP is a valid if address, localhost, or any
            if ($_SERVER["SERVER_ADDR"] === $if_info["ipaddr"]) {
                return;
            } elseif ($_SERVER["SERVER_ADDR"] === $if_info["ipaddrv6"]) {
                return;
            } elseif (in_array($_SERVER["SERVER_ADDR"], ["::1", "127.0.0.1", "localhost"]) and $wif === "localhost") {
                return;
            }elseif ($wif === "any") {
                return;
            }
        }

        # Return 444 response if we did not find a previous match
        $this->errors[] = APIResponse\get(6);
    }

}