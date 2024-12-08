/* Arduino library for pseudo randomized UUID and MAC address
 *
 * (c) 2022 stefan staub
 * Released under the MIT License
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/* uuid Tools following RFC4122
 * To initialise the pseudo random generators add an usefull input like AnalogIn(x)
 * At least you should use hardware chips with included serial and mac address like 
 * AT24MAC402/AT24MAC602 from microchip
*/

#ifndef ID_TOOLS_H
#define ID_TOOLS_H

#include "Arduino.h"

/**
 * @brief genertate a random UUID
 * 
 * @param uuid 
 * @param srnd start number for random function
 */
void generateUUID(uint8_t uuid[], unsigned int srnd) {
	srand(srnd);
	for (uint8_t i = 0; i < 16; i++) {
		uuid[i] = rand() %256;
		}
	uuid[6] = 0x40 | (0x0F & uuid[6]); // version 4 / random based
	uuid[8] = 0x80 | (0x3F & uuid[8]); // variant RFC4122
	}

/**
 * @brief genertate a random UUID
 * 
 * @param srnd start number for random function
 * @return uint8_t* uuid
 */
uint8_t* generateUUID(unsigned int srnd) {
	static uint8_t uuid[16];
	srand(srnd);
	for (uint8_t i = 0; i < 16; i++) {
		uuid[i] = rand() %256;
		}
	// add uuid variant and version
	uuid[6] = 0x40 | (0x0F & uuid[6]); // version 4 / random based
	uuid[8] = 0x80 | (0x3F & uuid[8]); // variant RFC4122
	return uuid;
	}

/**
 * @brief verify if UUID is RFC9562 conform
 * 
 * @param uuid 
 * @return true if variant and version ok
 * @return false 
 */
bool verifyUUID(uint8_t uuid[]) {
	if (uuid[6] >> 4 == 0x04 && uuid[8] >> 6 == 0x02) {
		return true;
		}
	else {
		return false;
		}
	}

/**
 * @brief converts the UUID to a string
 * 
 * @param uuid 
 * @param uuidString 
 */
void printUUID(uint8_t uuid[], char uuidString[]) {
	sprintf(uuidString, "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
	uuid[0], uuid[1], uuid[2], uuid[3], uuid[4], uuid[5], uuid[6], uuid[7],
	uuid[8], uuid[9], uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15]);
	}

/**
 * @brief converts the uuid / UUID to a string
 * 
 * @param uuid 
 * @return char* uuid tring 
 */
char* printUUID(uint8_t uuid[]) {
	static char uuidString[70];
	sprintf(uuidString, "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
	uuid[0], uuid[1], uuid[2], uuid[3], uuid[4], uuid[5], uuid[6], uuid[7],
	uuid[8], uuid[9], uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15]);
	return uuidString;
	}

/**
 * @brief generate a random mac address for local use
 * 
 * @param mac 
 * @param srnd start number for random function
 */
void generateMAC(uint8_t mac[], unsigned int srnd) {
	srand(srnd);
	for (uint8_t i = 0; i < 6; i++) {
		mac[i] = rand() %256;
		}
	mac[0] &= ~(1 << 0); // unicast = 0, multicast / broadcast = 1
	mac[0] |= 1 << 1; // local = 1, universal = 0
	mac[0] &= ~(1 << 2) & ~(1 << 3); // administratively assigned
	}

/**
 * @brief generate a random mac address for local use
 * 
 * @param srnd start number for random function
 * @return uint8_t* MAC address
 */
uint8_t* generateMAC(unsigned int srnd) {
	static uint8_t mac[6];
	srand(srnd);
	for (uint8_t i = 0; i < 6; i++) {
		mac[i] = rand() %256;
		}
	mac[0] &= ~(1 << 0); // unicast = 0, multicast / broadcast = 1
	mac[0] |= 1 << 1; // local = 1, universal = 0
	mac[0] &= ~(1 << 2) & ~(1 << 3); // administratively assigned
	return mac;
	}

/**
 * @brief converts the MAC to a string
 * 
 * @param mac 
 * @param macString 
 */
void printMAC(uint8_t mac[], char macString[]) {
	sprintf(macString, "%02X:%02X:%02X:%02X:%02X:%02X",
	mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	}

/**
 * @brief converts the MAC to a string
 * 
 * @param mac 
 * @return char* MAC string
 */
char* printMAC(uint8_t mac[]) {
	static char macString[31];
	sprintf(macString, "%02X:%02X:%02X:%02X:%02X:%02X",
	mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	return macString;
	}

#endif
