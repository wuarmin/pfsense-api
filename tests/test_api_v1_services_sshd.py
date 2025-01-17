# Copyright 2022 Jared Hendrickson
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import e2e_test_framework

class APIE2ETestServicesSSHd(e2e_test_framework.APIE2ETest):
    uri = "/api/v1/services/sshd"
    get_tests = [{"name": "Read SSHd configuration"}]
    put_tests = [
        {
            "name": "Update and disable SSHd",
            "payload": {
                "enable": False,
                "sshdkeyonly": "both",
                "sshdagentforwarding": False,
                "port": 2222
            }
        },
        {
            "name": "Update and enabled SSHd",
            "payload": {
                "enable": True,
                "sshdkeyonly": "disabled",
                "sshdagentforwarding": True,
                "port": 22
            }
        }
    ]

APIE2ETestServicesSSHd()
