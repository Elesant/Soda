#ifndef _SODA_H_
#define _SODA_H_

#include <uC++.h>
#include <uFuture.h>
#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>

#define Args unsigned int 0

struct ConfigParms 
{
    unsigned int sodaCost;                      // MSRP per bottle
    unsigned int numStudents;                   // number of students to create
    unsigned int maxPurchases;                  // maximum number of bottles a student purchases
    unsigned int numVendingMachines;            // number of vending machines
    unsigned int maxStockPerFlavour;            // maximum number of bottles of each flavour stocked
    unsigned int maxShippedPerFlavour;          // number of bottles of each flavour in a shipment
    unsigned int timeBetweenShipments;          // length of time between shipment pickup
    unsigned int parentalDelay;                 // length of time between cash deposits
    unsigned int numCouriers;                   // number of couriers in the pool
};

void processConfigFile(const char *configFile, ConfigParms &cparms);

// Forward declare
_Monitor Printer;
_Monitor Bank;
_Task NameServer;
_Task WATCardOffice;

_Task Student 
{
    void main();
  public:
    Student(Printer &prt, NameServer &nameServer, WATCardOffice &cardOffice, unsigned int id, unsigned int maxPurchases);
};

class WATCard 
{
    WATCard(const WATCard &);			// prevent copying
    WATCard &operator=(const WATCard &);
    int balance;
  public:
    WATCard();
    void deposit(unsigned int amount);
    void withdraw(unsigned int amount);
    unsigned int getBalance();
};
typedef Future_ISM<WATCard*> FWATCard;		// future WATCard pointer

_Task WATCardOffice 
{
    struct Job // marshalled arguments and return future
    {				
		Args args;					// call arguments (YOU DEFINE "Args")
		FWATCard result;			// return future
		Job(Args args) : args(args) {}
    };
    _Task Courier {  };				// communicates with bank
  
  Printer &prt;
  Bank &bank;
  unsigned int numCouriers;  
  void main();
  public:
    _Event Lost {};
    WATCardOffice(Printer &prt, Bank &bank, unsigned int numCouriers);
    FWATCard create(unsigned int sid, unsigned int amount, WATCard *&card);
    FWATCard transfer(unsigned int sid, unsigned int amount, WATCard *card);
    Job *requestWork();
};

_Monitor Bank 
{
  unsigned int numStudents;
  int *bankAccounts;
  public:
    Bank(unsigned int numStudents);
    void deposit(unsigned int id, unsigned int amount);
    void withdraw(unsigned int id, unsigned int amount);
};

_Task Parent 
{
  Printer &prt;
  Bank &bank;
  unsigned int numStudents;
  unsigned int parentalDelay;
  void main();
  public:
    Parent(Printer &prt, Bank &bank, unsigned int numStudents, unsigned int parentalDelay);
    ~Parent();
};

_Task VendingMachine 
{
    void main();
  public:
    enum Flavours {COKE, PEPSI, ROOTBEER}; 			// flavours of soda (YOU DEFINE)
    enum Status {BUY, STOCK, FUNDS};				// purchase status: successful buy, out of stock, insufficient funds
    VendingMachine(Printer &prt, NameServer &nameServer, unsigned int id, unsigned int sodaCost, unsigned int maxStockPerFlavour);
    Status buy(Flavours flavour, WATCard &card);
    unsigned int *inventory();
    void restocked();
    _Nomutex unsigned int cost();
    _Nomutex unsigned int getId();
};

_Task NameServer 
{
    void main();
  public:
    NameServer(Printer &prt, unsigned int numVendingMachines, unsigned int numStudents);
    void VMregister(VendingMachine *vendingmachine);
    VendingMachine *getMachine(unsigned int id);
    VendingMachine **getMachineList();
};

_Task BottlingPlant 
{
    void main();
  public:
    BottlingPlant(Printer &prt, NameServer &nameServer, unsigned int numVendingMachines, 
    	unsigned int maxShippedPerFlavour, unsigned int maxStockPerFlavour, unsigned int timeBetweenShipments);
    bool getShipment(unsigned int cargo[]);
};

_Task Truck 
{
    void main();
  public:
    Truck(Printer &prt, NameServer &nameServer, BottlingPlant &plant, unsigned int numVendingMachines, unsigned int maxStockPerFlavour);
};

_Monitor Printer 
{
  public:
    enum Kind {Parent, WATCardOffice, NameServer, Truck, BottlingPlant, Student, Vending, Courier};
    Printer(unsigned int numStudents, unsigned int numVendingMachines, unsigned int numCouriers);
    void print(Kind kind, char state);
    void print(Kind kind, char state, int value1);
    void print(Kind kind, char state, int value1, int value2);
    void print(Kind kind, unsigned int lid, char state);
    void print(Kind kind, unsigned int lid, char state, int value1);
    void print(Kind kind, unsigned int lid, char state, int value1, int value2);
};

_Monitor PRNG 
{
  public:
    PRNG( unsigned int seed = 1009 ) { srand( seed ); }
    void seed( unsigned int seed ) { srand( seed ); }
    unsigned int operator()() { return rand(); }
}; // _Monitor PRNG

extern PRNG r; // Global random generator

#endif // _SODA_H_
