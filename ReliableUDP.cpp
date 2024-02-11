/*
	Reliability and Flow Control Example
	From "Networking for Game Programmers" - http://www.gaffer.org/networking-for-game-programmers
	Author: Glenn Fiedler <gaffer@gaffer.org>
*/

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "FileHandler.h"    // included File handler file for handling the files in this code
#include "Net.h"


#pragma warning(disable : 4996)

#include "md5.h"

#define MD5_HASH_SIZE 33
//#define SHOW_ACKS

using namespace std;
using namespace net;

const int ServerPort = 30000;
const int ClientPort = 30001;
const int ProtocolId = 0x11223344;
const float DeltaTime = 1.0f / 30.0f;
const float SendRate = 1.0f / 30.0f;
const float TimeOut = 10.0f;
const int PacketSize = 256;

class FlowControl
{
public:

	FlowControl()
	{
		printf("flow control initialized\n");
		Reset();
	}

	void Reset()
	{
		mode = Bad;
		penalty_time = 4.0f;
		good_conditions_time = 0.0f;
		penalty_reduction_accumulator = 0.0f;
	}

	void Update(float deltaTime, float rtt)
	{
		const float RTT_Threshold = 250.0f;

		if (mode == Good)
		{
			if (rtt > RTT_Threshold)
			{
				printf("*** dropping to bad mode ***\n");
				mode = Bad;
				if (good_conditions_time < 10.0f && penalty_time < 60.0f)
				{
					penalty_time *= 2.0f;
					if (penalty_time > 60.0f)
						penalty_time = 60.0f;
					printf("penalty time increased to %.1f\n", penalty_time);
				}
				good_conditions_time = 0.0f;
				penalty_reduction_accumulator = 0.0f;
				return;
			}

			good_conditions_time += deltaTime;
			penalty_reduction_accumulator += deltaTime;

			if (penalty_reduction_accumulator > 10.0f && penalty_time > 1.0f)
			{
				penalty_time /= 2.0f;
				if (penalty_time < 1.0f)
					penalty_time = 1.0f;
				printf("penalty time reduced to %.1f\n", penalty_time);
				penalty_reduction_accumulator = 0.0f;
			}
		}

		if (mode == Bad)
		{
			if (rtt <= RTT_Threshold)
				good_conditions_time += deltaTime;
			else
				good_conditions_time = 0.0f;

			if (good_conditions_time > penalty_time)
			{
				printf("*** upgrading to good mode ***\n");
				good_conditions_time = 0.0f;
				penalty_reduction_accumulator = 0.0f;
				mode = Good;
				return;
			}
		}
	}

	float GetSendRate()
	{
		return mode == Good ? 30.0f : 10.0f;
	}

private:

	enum Mode
	{
		Good,
		Bad
	};

	Mode mode;
	float penalty_time;
	float good_conditions_time;
	float penalty_reduction_accumulator;
};



/**
 * @brief Check MD5 Hashes for File Integrity.
 *
 * This function reads the content of a file, computes its MD5 hash, and compares it
 * with a provided MD5 hash for integrity verification.
 *
 * @param fileName Path to the file to be checked.
 * @param fileSize Size of the file.
 * @param md5Hash The expected MD5 hash to compare against.
 * @return True if the computed MD5 hash matches the expected hash, false otherwise.
 */
bool checkMD5Hashes(char* fileName, int fileSize, char* md5Hash)
{
	FILE* inputFile = NULL;

	char* fileContents;
	fileContents = (char*)malloc(fileSize + 1);
	//This might be wrong
	fileContents[fileSize] = '\0';

	bool isHashEqual = false;

	string md5HashNum = "";
	string md5HashNumCompare = "";

	inputFile = fopen(fileName, "rb");
	if (inputFile == NULL)
	{
		printf("Error: Could not read the file\n");
		return 0;
	}

	//Maybe throw in a check in case the fileCotents is \0

	while (!feof(inputFile))
	{
		fread(fileContents, sizeof(char), fileSize, inputFile);
	}
	md5HashNum = md5(fileContents);

	if (md5HashNumCompare == md5Hash)
	{
		free(fileContents);

		isHashEqual = true;
	}
	else
	{
		free(fileContents);

		isHashEqual = false;
	}
	fclose(inputFile);
	return isHashEqual;
}


int main(int argc, char* argv[])
{
	//DATA FIELDS
	FILE* inputFile = NULL;
	char fileName[MD5_HASH_SIZE];
	char* fileContent;
	char* MD5Hash;
	int sizeOfInputFile = 0;
	int packetStage = 0;
	int packetStageServer = 0;
	string hashMD5String;
	int sendFileSize = 0;
	int sizeOfFileName = 0;
	FILE* outputFile = NULL;
	char receivedFileName[MD5_HASH_SIZE];
	char hashMD5Check[MD5_HASH_SIZE];
	unsigned char packet[PacketSize];
	unsigned char serverPacket[MD5_HASH_SIZE];
	// parse command line
	if (argc >= 3)
	{
		strcpy(fileName, argv[2]);
	}
	//FileHandler fileHandler;   // created a object of FileHandler class 

	enum Mode
	{
		Client,
		Server
	};

	Mode mode = Server;
	Address address;

	if (argc >= 2)
	{
		int a, b, c, d;
#pragma warning(suppress : 4996)
		if (sscanf(argv[1], "%d.%d.%d.%d", &a, &b, &c, &d))
		{
			mode = Client;
			address = Address(a, b, c, d, ServerPort);
		}
	}

	// initialize

	if (!InitializeSockets())
	{
		printf("failed to initialize sockets\n");
		return 1;
	}

	ReliableConnection connection(ProtocolId, TimeOut);

	const int port = mode == Server ? ServerPort : ClientPort;

	if (!connection.Start(port))
	{
		printf("could not start connection on port %d\n", port);
		return 1;
	}

	if (mode == Client)
	{


		inputFile = fopen(fileName, "rb");
		if (inputFile == NULL)
		{
			printf("ERROR: Cannot open file!\n");
		}
		else {
			fseek(inputFile, 0, SEEK_END);
			sizeOfInputFile = ftell(inputFile);
			fseek(inputFile, 0, SEEK_SET);

			fileContent = (char*)malloc(sizeOfInputFile + 1);
			fileContent[sizeOfInputFile] = 0;

			while (!feof(inputFile))
			{
				fread(fileContent, sizeof(char), sizeOfInputFile, inputFile);
			}
			hashMD5String = md5(fileContent);
			//maybe change


			free(fileContent);
			fclose(inputFile);
		}
	}


	if (mode == Mode::Client)
		connection.Connect(address);
	else
		connection.Listen();

	bool connected = false;
	float sendAccumulator = 0.0f;
	float statsAccumulator = 0.0f;

	FlowControl flowControl;

	while (true)
	{
		// update flow control

		if (connection.IsConnected())
			flowControl.Update(DeltaTime, connection.GetReliabilitySystem().GetRoundTripTime() * 1000.0f);

		const float sendRate = flowControl.GetSendRate();

		// detect changes in connection state

		if (mode == Server && connected && !connection.IsConnected())
		{
			// Here I will implement receiving the metadata of file when server is finally connected to the client
			/*
			* After receiving the metadata I will write out the received pieces
			* finally I will verify the integrity of the file after receiving the file's metadata and content if its correct then I will send the file for acknowledgement
			*/

			flowControl.Reset();
			printf("reset flow control\n");
			connected = false;
		}

		if (!connected && connection.IsConnected())
		{
			printf("client connected to server\n");
			connected = true;
		}

		if (!connected && connection.ConnectFailed())
		{
			printf("connection failed\n");
			break;
		}

		// send and receive packets

		sendAccumulator += DeltaTime;

		//This will be handling sending and receving PACKETS/INFORMATION!
		if (mode == Client) {
			inputFile = fopen(fileName, "rb");
			if (inputFile == NULL) {
				printf("ERROR: Opening file\n");
				return 0;
			}

			while (sendAccumulator > 1.0f / sendRate) {
				sizeOfFileName = strlen(fileName);

				memset(packet, 0, sizeof(packet));

				if (packetStage == 0) {
					sizeOfInputFile = sizeof(fileName);
					packetStage++;
					memcpy(packet, fileName, sizeOfFileName);
					printf("Transmitting file name: %s (stage 1)\n", fileName);
				}
				else if (packetStage == 1) {
					packetStage++;
					memcpy(packet, &sizeOfInputFile, sizeOfFileName);
					printf("Transmitting file size: %d (stage 2)\n", sizeOfInputFile);
				}
				else if (packetStage == 2) {
					memcpy(packet, hashMD5String.c_str(), MD5_HASH_SIZE);
					printf("Transmitting file hash: %s (stage 3)\n ", hashMD5String.c_str());
					packetStage++;
				}
				else
				{
					sizeOfInputFile = fread(packet, sizeof(char), PacketSize, inputFile);
					if (sizeOfInputFile == 0) {

						break;
					}
					sizeOfInputFile--;
					printf("Transmitting file contents: %s \n", packet);

				}

				connection.SendPacket(packet, sizeof(packet));
				sendAccumulator -= 1.0f / sendRate;
			}
		}
		while (true)
		{

			memset(serverPacket, 0, sizeof(serverPacket));
			int bytes = connection.ReceivePacket(serverPacket, sizeof(serverPacket));

			if (bytes == 0)
			{
				// No more data to read, break out of the loop
				break;
			}


			if (mode == Server) {
				if (packetStageServer == 0)
				{
					char fileNameOutput[256];
					memcpy(fileNameOutput, serverPacket, bytes);
					printf("Received file name: %s\n", fileNameOutput);
					outputFile = fopen(fileNameOutput, "wb");
					if (outputFile == NULL)
					{
						printf("ERROR: File cannot be Opened!\n");
					}
					packetStageServer++;
				}
				else if (packetStageServer == 1)
				{
					memcpy(&sendFileSize, serverPacket, bytes);
					printf("Received file size: %i\n", sendFileSize);
					packetStageServer++;
				}
				else if (packetStageServer == 2)
				{
					memcpy(hashMD5Check, serverPacket, bytes);
					printf("Received file hash: %s\n", hashMD5Check);
					packetStageServer++;
				}
				else
				{
					char printOut[200];
					memcpy(printOut, serverPacket, bytes);
					printf("Received file data: %s\n", printOut);
					if (fwrite(serverPacket, sizeof(char), bytes, outputFile) == 0)
					{
						printf("ERROR: Writing to file\n");
						return 0;
					}
				}
			}
		}



#ifdef SHOW_ACKS
		unsigned int* acks = NULL;
		int ack_count = 0;
		connection.GetReliabilitySystem().GetAcks(&acks, ack_count);
		if (ack_count > 0)
		{
			printf("acks: %d", acks[0]);
			for (int i = 1; i < ack_count; ++i)
				printf(",%d", acks[i]);
			printf("\n");
		}
#endif

		// update connection

		connection.Update(DeltaTime);

		// show connection stats

		statsAccumulator += DeltaTime;

		while (statsAccumulator >= 0.25f && connection.IsConnected())
		{
			float rtt = connection.GetReliabilitySystem().GetRoundTripTime();

			unsigned int sent_packets = connection.GetReliabilitySystem().GetSentPackets();
			unsigned int acked_packets = connection.GetReliabilitySystem().GetAckedPackets();
			unsigned int lost_packets = connection.GetReliabilitySystem().GetLostPackets();

			float sent_bandwidth = connection.GetReliabilitySystem().GetSentBandwidth();
			float acked_bandwidth = connection.GetReliabilitySystem().GetAckedBandwidth();

			printf("rtt %.1fms, sent %d, acked %d, lost %d (%.1f%%), sent bandwidth = %.1fkbps, acked bandwidth = %.1fkbps\n",
				rtt * 1000.0f, sent_packets, acked_packets, lost_packets,
				sent_packets > 0.0f ? (float)lost_packets / (float)sent_packets * 100.0f : 0.0f,
				sent_bandwidth, acked_bandwidth);

			statsAccumulator -= 0.25f;
		}

		net::wait(DeltaTime);
	}

	ShutdownSockets();

	return 0;


}



