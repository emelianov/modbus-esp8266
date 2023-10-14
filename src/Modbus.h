/*
    Modbus Library for Arduino
    Core functions
    Copyright (C) 2014 Andr� Sarmento Barbosa
                  2017-2022 Alexander Emelianov (a.m.emelianov@gmail.com)
*/
#pragma once
#include "ModbusSettings.h"
#include "Arduino.h"
#if defined(MODBUS_USE_STL)
 #include <vector>
 #include <algorithm>
 #include <functional>
 #include <memory>
#else
 #include "darray.h"
#endif

static inline uint16_t __swap_16(uint16_t num) { return (num >> 8) | (num << 8); }

#define COIL(n) (TAddress){TAddress::COIL, n}
#define ISTS(n) (TAddress){TAddress::ISTS, n}
#define IREG(n) (TAddress){TAddress::IREG, n}
#define HREG(n) (TAddress){TAddress::HREG, n}
#define NULLREG (TAddress){TAddress::NONE, 0xFFFF}
#define BIT_VAL(v) (v?0xFF00:0x0000)
#define BIT_BOOL(v) (v==0xFF00)
#define COIL_VAL(v) (v?0xFF00:0x0000)
#define COIL_BOOL(v) (v==0xFF00)
#define ISTS_VAL(v) (v?0xFF00:0x0000)
#define ISTS_BOOL(v) (v==0xFF00)

// For deprecated (v1.xx) onSet/onGet format compatibility
#define cbDefault nullptr

struct TRegister;
#if defined(MODBUS_USE_STL)
typedef std::function<uint16_t(TRegister* reg, uint16_t val)> cbModbus; // Callback function Type
#else
typedef uint16_t (*cbModbus)(TRegister* reg, uint16_t val); // Callback function Type
#endif

struct TAddress {
    enum RegType {COIL, ISTS, IREG, HREG, NONE = 0xFF};
    RegType type;
    uint16_t address;
    bool operator==(const TAddress &obj) const { // TAddress == TAddress
	    return type == obj.type && address == obj.address;
	}
    bool operator!=(const TAddress &obj) const { // TAddress != TAddress
        return type != obj.type || address != obj.address;
    }
    TAddress& operator++() {     // ++TAddress
        address++;
        return *this;
    }
    TAddress  operator++(int) {  // TAddress++
        TAddress result(*this);
         ++(*this);
        return result;
    }
    TAddress& operator+=(const int& inc) {  // TAddress += integer
        address += inc;
        return *this;
    }
    const TAddress operator+(const int& inc) const {    // TAddress + integer
        TAddress result(*this);
        result.address += inc;
        return result;
    }
    bool isCoil() {
       return type == COIL;
    }
    bool isIsts() {
       return type == ISTS;
    }
    bool isIreg() {
        return type == IREG;
    }
    bool isHreg() {
        return type == HREG;
    }
};

struct TCallback {
    enum CallbackType {ON_SET, ON_GET};
    CallbackType type;
    TAddress    address;
    cbModbus    cb;
};

struct TRegister {
    TAddress    address;
    uint16_t value;
    bool operator ==(const TRegister &obj) const {
	    return address == obj.address;
	}
};

class Modbus {
    public:
        //Function Codes
        enum FunctionCode {
            FC_READ_COILS       = 0x01, // Read Coils (Output) Status
            FC_READ_INPUT_STAT  = 0x02, // Read Input Status (Discrete Inputs)
            FC_READ_REGS        = 0x03, // Read Holding Registers
            FC_READ_INPUT_REGS  = 0x04, // Read Input Registers
            FC_WRITE_COIL       = 0x05, // Write Single Coil (Output)
            FC_WRITE_REG        = 0x06, // Preset Single Register
            FC_DIAGNOSTICS      = 0x08, // Not implemented. Diagnostics (Serial Line only)
            FC_WRITE_COILS      = 0x0F, // Write Multiple Coils (Outputs)
            FC_WRITE_REGS       = 0x10, // Write block of contiguous registers
            FC_READ_FILE_REC    = 0x14, // Read File Record
            FC_WRITE_FILE_REC   = 0x15, // Write File Record
            FC_MASKWRITE_REG    = 0x16, // Mask Write Register
            FC_READWRITE_REGS   = 0x17  // Read/Write Multiple registers
        };
        //Exception Codes
        //Custom result codes used internally and for callbacks but never used for Modbus response
        enum ResultCode {
            EX_SUCCESS              = 0x00, // Custom. No error
            EX_ILLEGAL_FUNCTION     = 0x01, // Function Code not Supported
            EX_ILLEGAL_ADDRESS      = 0x02, // Output Address not exists
            EX_ILLEGAL_VALUE        = 0x03, // Output Value not in Range
            EX_SLAVE_FAILURE        = 0x04, // Slave or Master Device Fails to process request
            EX_ACKNOWLEDGE          = 0x05, // Not used
            EX_SLAVE_DEVICE_BUSY    = 0x06, // Not used
            EX_MEMORY_PARITY_ERROR  = 0x08, // Not used
            EX_PATH_UNAVAILABLE     = 0x0A, // Not used
            EX_DEVICE_FAILED_TO_RESPOND = 0x0B, // Not used
            EX_GENERAL_FAILURE      = 0xE1, // Custom. Unexpected master error
            EX_DATA_MISMACH         = 0xE2, // Custom. Inpud data size mismach
            EX_UNEXPECTED_RESPONSE  = 0xE3, // Custom. Returned result doesn't mach transaction
            EX_TIMEOUT              = 0xE4, // Custom. Operation not finished within reasonable time
            EX_CONNECTION_LOST      = 0xE5, // Custom. Connection with device lost
            EX_CANCEL               = 0xE6, // Custom. Transaction/request canceled
            EX_PASSTHROUGH          = 0xE7, // Custom. Raw callback. Indicate to normal processing on callback exit
            EX_FORCE_PROCESS        = 0xE8  // Custom. Raw callback. Indicate to force processing on callback exit
        };
        union RequestData {
            struct {
                TAddress reg;
                uint16_t regCount;
            };
            struct {
                TAddress regRead;
                uint16_t regReadCount;
                TAddress regWrite;
                uint16_t regWriteCount;
            };
            struct {
                TAddress regMask;
                uint16_t andMask;
                uint16_t orMask;
            };
            RequestData(TAddress r1, uint16_t c1) {
                reg = r1;
                regCount = c1;
            };
            RequestData(TAddress r1, uint16_t c1, TAddress r2, uint16_t c2) {
                regRead = r1;
                regReadCount = c1;
                regWrite = r2;
                regWriteCount = c2;
            };
            RequestData(TAddress r1, uint16_t m1, uint16_t m2) {
                regMask = r1;
                andMask = m1;
                orMask = m2;
            };
        };

	    struct frame_arg_t {
            bool to_server;
            union {
		        uint8_t slaveId;
		        struct {
			        uint8_t unitId;
			        uint32_t ipaddr;
			        uint16_t transactionId;
		        };
            };
            frame_arg_t(uint8_t s, bool m = false) {
                slaveId = s;
                to_server = m;
            };
            frame_arg_t(uint8_t u, uint32_t a, uint16_t t, bool m = false) {
                unitId = u;
                ipaddr = a;
                transactionId = t;
                to_server = m;
            };
	    };

        ~Modbus();

        bool cbEnable(const bool state = true);
        bool cbDisable();

    private:
	    ResultCode readBits(TAddress startreg, uint16_t numregs, FunctionCode fn);
	    ResultCode readWords(TAddress startreg, uint16_t numregs, FunctionCode fn);
        
        bool setMultipleBits(uint8_t* frame, TAddress startreg, uint16_t numoutputs);
        bool setMultipleWords(uint16_t* frame, TAddress startreg, uint16_t numoutputs);
        
        void getMultipleBits(uint8_t* frame, TAddress startreg, uint16_t numregs);
        void getMultipleWords(uint16_t* frame, TAddress startreg, uint16_t numregs);

        void bitsToBool(bool* dst, uint8_t* src, uint16_t numregs);
        void boolToBits(uint8_t* dst, bool* src, uint16_t numregs);
    
    protected:
        //Reply Types
        enum ReplyCode {
            REPLY_OFF            = 0x01,
            REPLY_ECHO           = 0x02,
            REPLY_NORMAL         = 0x03,
            REPLY_ERROR          = 0x04,
            REPLY_UNEXPECTED     = 0x05
        };
        #if defined(MODBUS_USE_STL)
        #if defined(MODBUS_GLOBAL_REGS)
        static std::vector<TRegister> _regs;
        static std::vector<TCallback> _callbacks;
        #if defined(MODBUS_FILES)
        static std::function<ResultCode(FunctionCode, uint16_t, uint16_t, uint16_t, uint8_t*)> _onFile;
        #endif
        #else
        std::vector<TRegister> _regs;
        std::vector<TCallback> _callbacks;
        #if defined(MODBUS_FILES)
        std::function<ResultCode(FunctionCode, uint16_t, uint16_t, uint16_t, uint8_t*)> _onFile;
        #endif
        #endif
        #else
        #if defined(MODBUS_GLOBAL_REGS)
        static DArray<TRegister, 1, 1> _regs;
        static DArray<TCallback, 1, 1> _callbacks;
        #if defined(MODBUS_FILES)
        static ResultCode (*_onFile)(FunctionCode, uint16_t, uint16_t, uint16_t, uint8_t*);
        #endif
        #else
        DArray<TRegister, 1, 1> _regs;
        DArray<TCallback, 1, 1> _callbacks;
        #if defined(MODBUS_FILES)
        ResultCode (*_onFile)(FunctionCode, uint16_t, uint16_t, uint16_t, uint8_t*)= nullptr;
        #endif
        #endif
        #endif

        uint8_t*  _frame = nullptr;
        uint16_t  _len = 0;
        uint8_t   _reply = 0;
        bool cbEnabled = true;
        uint16_t callback(TRegister* reg, uint16_t val, TCallback::CallbackType t);
        virtual TRegister* searchRegister(TAddress addr);
        void exceptionResponse(FunctionCode fn, ResultCode excode); // Fills _frame with response
        void successResponse(TAddress startreg, uint16_t numoutputs, FunctionCode fn);  // Fills frame with response
        // Use `successResponse` instead
        [[deprecated]]
        void successResponce(TAddress startreg, uint16_t numoutputs, FunctionCode fn) {
            successResponse(startreg, numoutputs, fn);
        }
        void slavePDU(uint8_t* frame);    //For Slave
        void masterPDU(uint8_t* frame, uint8_t* sourceFrame, TAddress startreg, uint8_t* output = nullptr);   //For Master
        // frame - data received form slave
        // sourceFrame - data have sent fo slave
        // startreg - local register to start put data to
        // output - if not null put data to the buffer insted local registers. output assumed to by array of uint16_t or boolean

        bool readSlave(uint16_t address, uint16_t numregs, FunctionCode fn);
        bool writeSlaveBits(TAddress startreg, uint16_t to, uint16_t numregs, FunctionCode fn, bool* data = nullptr);
        bool writeSlaveWords(TAddress startreg, uint16_t to, uint16_t numregs, FunctionCode fn, uint16_t* data = nullptr);
        // startreg - local register to get data from
        // to - slave register to write data to
        // numregs - number of registers
        // fn - Modbus function
        // data - if null use local registers. Otherwise use data from array to erite to slave
        bool removeOn(TCallback::CallbackType t, TAddress address, cbModbus cb = nullptr, uint16_t numregs = 1);
    public:
        bool addReg(TAddress address, uint16_t value = 0, uint16_t numregs = 1);
        bool Reg(TAddress address, uint16_t value);
        uint16_t Reg(TAddress address);
        bool removeReg(TAddress address, uint16_t numregs = 1);
        bool addReg(TAddress address, uint16_t* value, uint16_t numregs = 1);
        bool Reg(TAddress address, uint16_t* value, uint16_t numregs = 1);

        bool onGet(TAddress address, cbModbus cb = nullptr, uint16_t numregs = 1);
        bool onSet(TAddress address, cbModbus cb = nullptr, uint16_t numregs = 1);
        bool removeOnSet(TAddress address, cbModbus cb = nullptr, uint16_t numregs = 1);
        bool removeOnGet(TAddress address, cbModbus cb = nullptr, uint16_t numregs = 1);

        virtual uint32_t eventSource() {return 0;}
        #if defined(MODBUS_USE_STL)
        typedef std::function<ResultCode(FunctionCode, const RequestData)> cbRequest; // Callback function Type
        typedef std::function<ResultCode(uint8_t*, uint8_t, void*)> cbRaw; // Callback function Type
        #else
        typedef ResultCode (*cbRequest)(FunctionCode fc, const RequestData data); // Callback function Type
        typedef ResultCode (*cbRaw)(uint8_t*, uint8_t, void*); // Callback function Type
        #endif

    protected:
        cbRaw _cbRaw = nullptr;
        static ResultCode _onRequestDefault(FunctionCode fc, const RequestData data);
        cbRequest _onRequest = _onRequestDefault;
    public:
        bool onRaw(cbRaw cb = nullptr);
        bool onRequest(cbRequest cb = _onRequestDefault);
    #if defined (MODBUSAPI_OPTIONAL)
    protected:
        cbRequest _onRequestSuccess = _onRequestDefault;
    public:
        bool onRequestSuccess(cbRequest cb = _onRequestDefault);
    #endif

    #if defined(MODBUS_FILES)
    public:
        #if defined(MODBUS_USE_STL)
        bool onFile(std::function<ResultCode(FunctionCode, uint16_t, uint16_t, uint16_t, uint8_t*)>);
        #else
        bool onFile(ResultCode (*cb)(FunctionCode, uint16_t, uint16_t, uint16_t, uint8_t*));
        #endif
    private:
        ResultCode fileOp(FunctionCode fc, uint16_t fileNum, uint16_t recNum, uint16_t recLen, uint8_t* frame);
    protected:
        bool readSlaveFile(uint16_t* fileNum, uint16_t* startRec, uint16_t* len, uint8_t count, FunctionCode fn);
        // fileNum - sequental array of files numbers to read
        // startRec - array of strart records for each file
        // len - array of counts of records to read in terms of register size (2 bytes) for each file
        // count - count of records to be compose in the single request
        // fn - Modbus function. Assumed to be 0x14
        bool writeSlaveFile(uint16_t* fileNum, uint16_t* startRec, uint16_t* len, uint8_t count, FunctionCode fn, uint8_t* data);
        // fileNum - sequental array of files numbers to read
        // startRec - array of strart records for each file
        // len - array of counts of records to read in terms of register size (2 bytes) for each file
        // count - count of records to be compose in the single request
        // fn - Modbus function. Assumed to be 0x15
        // data - sequental set of data records
    #endif

};

#if defined(MODBUS_USE_STL)
typedef std::function<bool(Modbus::ResultCode, uint16_t, void*)> cbTransaction; // Callback skeleton for requests
#else
typedef bool (*cbTransaction)(Modbus::ResultCode event, uint16_t transactionId, void* data); // Callback skeleton for requests
#endif
//typedef Modbus::ResultCode (*cbRequest)(Modbus::FunctionCode func, TRegister* reg, uint16_t regCount); // Callback function Type
#if defined(MODBUS_FILES)
// Callback skeleton for file read/write
#if defined(MODBUS_USE_STL)
typedef std::function<Modbus::ResultCode(Modbus::FunctionCode, uint16_t, uint16_t, uint16_t, uint8_t*)> cbModbusFileOp;
#else
typedef Modbus::ResultCode (*cbModbusFileOp)(Modbus::FunctionCode func, uint16_t fileNum, uint16_t recNumber, uint16_t recLength, uint8_t* frame);
#endif
#endif

#if defined(ARDUINO_SAM_DUE_STL)
// Arduino Due STL workaround
namespace std {
    void __throw_bad_function_call();
}
#endif