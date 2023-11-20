/*
Author: Abby McCollam
Class: ECE4122 Section A
Last Date Modified: 11/26/23
Description: Server prompting user for commands to execute
*/

//headers
#include <SFML/Network.hpp>
#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <omp.h>
#include <algorithm>
#include <list>
#include <cstdlib>

using namespace std;

//creating instances of classes
list<sf::TcpSocket*> clients;
sf::SocketSelector selector;
sf::TcpListener listener;
sf::Packet exitPacket;
sf::Packet packet;
sf::Packet packet77;
sf::Packet packet201;

struct tcpMessage //Data using following packet structure
{
    unsigned char nVersion;
    unsigned char nType;
    unsigned short nMsgLen;
    char chMsg[1000];
};

//initializing message structure
tcpMessage lastMessageReceived = { 102, 77, 1, ' '};
string command;

//acquiring and printing connected clients list
void printConnectedClients()
{
    cout << "Number of Clients: " << clients.size() << endl;
    for (list<sf::TcpSocket*>::iterator it = clients.begin(); it != clients.end(); ++it)
    {
        sf::TcpSocket& client = **it;
        cout << "IP Address : " << client.getRemoteAddress() << " | Port : " << client.getRemotePort() << endl;
    }
}

void timeToExit() //exit function when q pressed
{
    lastMessageReceived.chMsg[0] = 'q';
    lastMessageReceived.nVersion = 1; //setting nVersion to 1 and it is read by client
    exitPacket << lastMessageReceived.nVersion << lastMessageReceived.nType << lastMessageReceived.nMsgLen << lastMessageReceived.chMsg; //exit packet
    for (list<sf::TcpSocket*>::iterator it = clients.begin(); it != clients.end(); ++it) //sending exit message to clients
    {
        sf::TcpSocket& client = **it;
        if (client.send(exitPacket) != sf::Socket::Done)
            continue;
        client.disconnect();
    }
    cout << "Goodbye." << endl;
    exit(0);
}

void readyForClient() //function for accepting new client connections
{
    sf::TcpSocket* socket = new sf::TcpSocket;
    if (listener.accept(*socket) != sf::Socket::Done)
    {
        cout << "Error" << endl;
        delete socket;
    }
    clients.push_back(socket);
    selector.add(*socket);
}

void processType201() //function for when version 201 chosen
{
    reverse(lastMessageReceived.chMsg, lastMessageReceived.chMsg + strlen(lastMessageReceived.chMsg)); //reverse order
    packet201 << lastMessageReceived.nVersion << lastMessageReceived.nType << lastMessageReceived.nMsgLen << lastMessageReceived.chMsg;
}

void processType77(const tcpMessage& message, sf::TcpSocket& client, const list<sf::TcpSocket*>& clientTemp) //function for when version 77 chosen
{
    packet77 << message.nVersion << message.nType << message.nMsgLen << message.chMsg;

    for (const auto& otherClient : clientTemp)
    {
        sf::TcpSocket& other = *otherClient;
        if (other.getRemotePort() != client.getRemotePort())
        {
            bool cont = true;
            while (cont)
            {
                if (other.send(packet) != sf::Socket::Done)
                {
                    continue;
                }
                cont = false;
            }
        }
    }
}

int main(int argc, char* argv[])
{
    if (argc != 2) //checking for proper number of input arguments
    {
        cout << "Usage: ./ServerTCP <port>" << endl;
        return 1;
    }

    unsigned short port = static_cast<unsigned short>(stoi(argv[1]));

    if (listener.listen(port) != sf::Socket::Done) //start listening to incoming connections
    {
        return 0;
    }

    //BEGIN OPEN MP
#pragma omp parallel sections
    {
#pragma omp section
        {
            while (true)
            {
                readyForClient(); //accepting clients
            }
        }
#pragma omp section
        {
            while (true)
            {
                cout << "Please enter command: ";
                cin >> command;

                //all options for server commands
                if (command == "msg")
                    cout << "Last Message: " << lastMessageReceived.chMsg << endl;
                else if (command == "clients")
                {
                    printConnectedClients();
                }
                else if (command == "exit")
                {
                    timeToExit();
                }
                else //error checking
                {
                    cout << "Please enter valid input." << endl;
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                }
            }
        }
#pragma omp section
        {
            while (true) //continuously handling events from connected clients
            {
                if (selector.wait(sf::milliseconds(10.f))) // waits for events on selector
                {
                    for (list<sf::TcpSocket*>::iterator it = clients.begin(); it != clients.end(); ++it) //iterating over all clients
                    {
                        sf::TcpSocket& client = **it;
                        if (selector.isReady(client))
                        {
                            if (client.receive(packet) == sf::Socket::Disconnected) //removes client if disconnected
                            {
                                selector.remove(client);
                                client.disconnect();
                                delete(&client);
                                clients.erase(it);
                                break;
                            }
                            packet >> lastMessageReceived.nVersion >> lastMessageReceived.nType >> lastMessageReceived.nMsgLen >> lastMessageReceived.chMsg; //receives packet from client

                            //handles message based on version number
                            if (lastMessageReceived.nVersion != 102) //version number 102
                            {
                                continue; //do nothing
                            }
                            else if (lastMessageReceived.nType == 201) //version number 201
                            {
                                processType201();
                                if (client.send(packet201) != sf::Socket::Done)
                                {
                                    continue;
                                }
                            }
                            else if (lastMessageReceived.nType == 77) //version number 77
                            {
                                processType77(lastMessageReceived, client, clients);
                            }
                            else if (lastMessageReceived.nType == 1) //version number 77
                            {
                                cout << "Connection closed from client." << endl;
                                exit(0);
                            }
                        }
                    }
                }
            }
        }
    }
    //closing connection
    cout << "Closed server. Goodbye." << endl;
    return 0;
}