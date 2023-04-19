#include <iostream>
#include <cstring>
#include <mosquitto.h>
#include <queue>




#define TIMEOUT 10000L
using namespace std;

std::queue<std::string> messageQueue;

bool isConnected = false;

void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg) {
    string message((const char *)msg->payload, msg->payloadlen);

    if (isConnected) {
        cout << message << std::endl;
    } else {
        messageQueue.push(message);
    }
}




void on_connect(struct mosquitto *mosq, void *obj, int rc) {
    if (rc == 0) {
        cout << "....Connected to MQTT broker!...." << std::endl << std::endl;
        isConnected = true;
        while (!messageQueue.empty()) {
            string message = messageQueue.front();
            cout << message << std::endl;
            messageQueue.pop();
        }
    } else {
        cout << "Failed to connect to MQTT broker: " << mosquitto_connack_string(rc) << std::endl;
    }
}


int main()
{
	struct mosquitto *mosq = NULL;

	// Initialize Mosquitto library
	mosquitto_lib_init();

	// Create a new Mosquitto client instance
	mosq = mosquitto_new(NULL, false, NULL);
	if (!mosq) {
		cerr << "Failed to create Mosquitto client instance" << std::endl;
		return 1;
	}

	// Set up callback functions
	// mosquitto_connect_callback_set(mosq, on_connect);
	mosquitto_message_callback_set(mosq, on_message);

	// Connect to MQTT broker : mosquitto_connect(mosq, HOST, PORT, KEEP_ALIVE)
	if (mosquitto_connect(mosq, "localhost", 1883, 60) != MOSQ_ERR_SUCCESS) {
		cerr << "Failed to connect to MQTT broker" << std::endl;
		mosquitto_destroy(mosq);
		return 1;
	}

	// Subscribe to "group-chat" topic : mosquitto_subscribe(mosq, , "topic-name", QOS)
	if (mosquitto_subscribe(mosq, NULL, "/sasken.group-chat", 1 )!= MOSQ_ERR_SUCCESS)
         {
 
		cerr << "Failed to subscribe to '/sasken.group-chat' topic" <<endl;
		mosquitto_destroy(mosq);
		return 1;
	}

	// Start Mosquitto network loop in a separate thread
	mosquitto_loop_start(mosq);

	string name;
	cout << "Enter your name: ";
	cin >> name;



	// join the group chat
	string join_msg = name + " has joined the group chat.";
	cout<<endl;
	mosquitto_publish(mosq, NULL, "/sasken.group-chat", join_msg.length(), join_msg.c_str(), 0, false);


	// Main loop


	while (true) 
	{
		//cout << "Enter message: ";
		string message;
		getline(cin, message);

		if (message.empty()) {
			continue;
		}

		if( strncmp("exit", message.c_str() , 4)==0){

			break;
		}

		// Publish message to "group-chat" topic

		int mid = 0;
		message = "[" + name + "]: " + message;

		if (mosquitto_publish(mosq, &mid, "/sasken.group-chat", message.size(), message.c_str(), 0, false) != MOSQ_ERR_SUCCESS) 
		{
			cerr << "Failed to publish message" << endl;
		}

	}

	// Clean up Mosquitto library
	mosquitto_lib_cleanup();
	return 0;
}



