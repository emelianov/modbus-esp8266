/*
    Modbus Library for Arduino
    Core functions
    Copyright (C) 2014 Andr� Sarmento Barbosa
                  2017-2021 Alexander Emelianov (a.m.emelianov@gmail.com)
*/
#include "Modbus.h"

#if defined(MODBUS_GLOBAL_REGS)
#if defined(MODBUS_USE_STL)
 std::vector<TRegister> Modbus::_regs;
 std::vector<TCallback> Modbus::_callbacks;
 #if defined(MODBUS_FILES) 
 std::function<Modbus::ResultCode(Modbus::FunctionCode, uint16_t, uint16_t, uint16_t, uint8_t*)> Modbus::_onFile;
 #endif
#else
 DArray<TRegister, 1, 1> Modbus::_regs;
 DArray<TCallback, 1, 1> Modbus::_callbacks;
 #if defined(MODBUS_FILES)
 cbModbusFileOp Modbus::_onFile = nullptr;
 #endif
#endif
#endif

uint16_t Modbus::callback(TRegister* reg, uint16_t val, TCallback::CallbackType t) {
#define MODBUS_COMPARE_CB [reg, t](TCallback& cb){return cb.address == reg->address && cb.type == t;}
    uint16_t newVal = val;
#if defined(MODBUS_USE_STL)
    std::vector<TCallback>::iterator it = _callbacks.begin();
    do {
        it = std::find_if(it, _callbacks.end(), MODBUS_COMPARE_CB);
        if (it != _callbacks.end()) {
            newVal = it->cb(reg, newVal);
            it++;
        }
    } while (it != _callbacks.end());
#else
    size_t r = 0; 
    do {
        r = _callbacks.find(MODBUS_COMPARE_CB, r);
        if (r < _callbacks.size())
            newVal = _callbacks[r].cb(reg, newVal);
        r++;
    } while (r < _callbacks.size());
#endif
    return newVal;
}

TRegister* Modbus::searchRegister(TAddress address) {
#define MODBUS_COMPARE_REG [address](TRegister& addr){return (addr.address == address);}
#if defined(MODBUS_USE_STL)
    std::vector<TRegister>::iterator it = std::find_if(_regs.begin(), _regs.end(), MODBUS_COMPARE_REG);
    if (it != _regs.end()) return &*it;
#else
    size_t r = _regs.find(MODBUS_COMPARE_REG);
    if (r < _regs.size()) return _regs.entry(r); 
#endif
    return nullptr;
}

bool Modbus::addReg(TAddress address, uint16_t value, uint16_t numregs) {
   #if defined(MODBUS_MAX_REGS)
    if (_regs.size() + numregs > MODBUS_MAX_REGS) return false;
   #endif
    if (0xFFFF - address.address < numregs)
        numregs = 0xFFFF - address.address;
    for (uint16_t i = 0; i < numregs; i++) {
        if (!searchRegister(address + i))
            _regs.push_back({address + i, value});
    }
    //std::sort(_regs.begin(), _regs.end());
    return true;
}

bool Modbus::Reg(TAddress address, uint16_t value) {
    TRegister* reg;
    reg = searchRegister(address); //search for the register address
    if (reg) { //if found then assign the register value to the new value.
        if (cbEnabled) {
            reg->value = callback(reg, value, TCallback::ON_SET);
        } else {
            reg->value = value;
        }
        return true;
    } else 
        return false;
}

uint16_t Modbus::Reg(TAddress address) {
    TRegister* reg;
    reg = searchRegister(address);
    if(reg)
        if (cbEnabled) {
            return callback(reg, reg->value, TCallback::ON_GET);
        } else {
            return reg->value;
        }
    else
        return 0;
}

bool Modbus::removeReg(TAddress address, uint16_t numregs) {
    TRegister* reg;
    bool atLeastOne = false;
    if (0xFFFF - address.address < numregs)
        numregs = 0xFFFF - address.address;
    for (uint16_t i = 0; i < numregs; i++) {
        reg = searchRegister(address + i);
        if (reg) {
            atLeastOne = true;
            removeOnSet(address + i);
            removeOnGet(address + i);
            #if defined(MODBUS_USE_STL)
            _regs.erase(std::remove( _regs.begin(), _regs.end(), *reg), _regs.end() );
            #else
            _regs.remove(_regs.find(MODBUS_COMPARE_REG));
            #endif
        }
    }
    return atLeastOne;
}

bool Modbus::addReg(TAddress address, uint16_t* value, uint16_t numregs) {
    if (0xFFFF - address.address < numregs)
        numregs = 0xFFFF - address.address;
	for (uint16_t k = 0; k < numregs; k++)
		addReg(address + k, value[k]);
	return true;
}

void Modbus::slavePDU(uint8_t* frame) {
    FunctionCode fcode  = (FunctionCode)frame[0];
    uint16_t field1 = (uint16_t)frame[1] << 8 | (uint16_t)frame[2];
    uint16_t field2 = (uint16_t)frame[3] << 8 | (uint16_t)frame[4];
    uint16_t field3 = 0;
    uint16_t field4 = 0;
    uint16_t bytecount_calc;
    uint16_t k;
    ResultCode ex;
    switch (fcode) {
        case FC_WRITE_REG:
            //field1 = reg, field2 = value
            ex = _onRequest(fcode, {HREG(field1), field2});
            if (ex != EX_SUCCESS) {
                exceptionResponse(fcode, ex);
                return;
            }
            if (!Reg(HREG(field1), field2)) { //Check Address and execute (reg exists?)
                exceptionResponse(fcode, EX_ILLEGAL_ADDRESS);
                return;
            }
            if (Reg(HREG(field1)) != field2) { //Check for failure
                exceptionResponse(fcode, EX_SLAVE_FAILURE);
                return;
            }
            _reply = REPLY_ECHO;
            _onRequestSuccess(fcode, {HREG(field1), field2});
        break;

        case FC_READ_REGS:
            //field1 = startreg, field2 = numregs, header len = 3
            ex = _onRequest(fcode, {HREG(field1), field2});
            if (ex != EX_SUCCESS) {
                exceptionResponse(fcode, ex);
                return;
            }
            ex = readWords(HREG(field1), field2, fcode);
            if (ex != EX_SUCCESS) {
                exceptionResponse(fcode, ex);
                return;
            }
            _onRequestSuccess(fcode, {HREG(field1), field2});
        break;

        case FC_WRITE_REGS:
            //field1 = startreg, field2 = numregs, frame[5] = data lenght, header len = 6
            ex = _onRequest(fcode, {HREG(field1), field2});
            if (ex != EX_SUCCESS) {
                exceptionResponse(fcode, ex);
                return;
            }
            if (field2 < 0x0001 || field2 > MODBUS_MAX_WORDS || 0xFFFF - field1 < field2 || frame[5] != 2 * field2) { //Check constrains
                exceptionResponse(fcode, EX_ILLEGAL_VALUE);
                return;
            }
            for (k = 0; k < field2; k++) { //Check Address (startreg...startreg + numregs)
                if (!searchRegister(HREG(field1) + k)) {
                    exceptionResponse(fcode, EX_ILLEGAL_ADDRESS);
                    return;
                }
            }
            if (!setMultipleWords((uint16_t*)(frame + 6), HREG(field1), field2)) {
                exceptionResponse(fcode, EX_SLAVE_FAILURE);
                return;
            }
            successResponce(HREG(field1), field2, fcode);
            _reply = REPLY_NORMAL;
            _onRequestSuccess(fcode, {HREG(field1), field2});
        break;

        case FC_READ_COILS:
            //field1 = startreg, field2 = numregs
            ex = _onRequest(fcode, {COIL(field1), field2});
            if (ex != EX_SUCCESS) {
                exceptionResponse(fcode, ex);
                return;
            }
            ex = readBits(COIL(field1), field2, fcode);
            if (ex != EX_SUCCESS) {
                exceptionResponse(fcode, ex);
                return;
            }
            _onRequestSuccess(fcode, {COIL(field1), field2});
        break;

        case FC_READ_INPUT_STAT:
            //field1 = startreg, field2 = numregs
            ex = _onRequest(fcode, {ISTS(field1), field2});
            if (ex != EX_SUCCESS) {
                exceptionResponse(fcode, ex);
                return;
            }
            ex = readBits(ISTS(field1), field2, fcode);
            if (ex != EX_SUCCESS) {
                exceptionResponse(fcode, ex);
                return;
            }
            _onRequestSuccess(fcode, {ISTS(field1), field2});
        break;

        case FC_READ_INPUT_REGS:
            //field1 = startreg, field2 = numregs
            ex = _onRequest(fcode, {IREG(field1), field2});
            if (ex != EX_SUCCESS) {
                exceptionResponse(fcode, ex);
                return;
            }
            ex = readWords(IREG(field1), field2, fcode);
            if (ex != EX_SUCCESS) {
                exceptionResponse(fcode, ex);
                return;
            }
            _onRequestSuccess(fcode, {IREG(field1), field2});
        break;

        case FC_WRITE_COIL:
            //field1 = reg, field2 = status, header len = 3
            ex = _onRequest(fcode, {COIL(field1), field2});
            if (ex != EX_SUCCESS) {
                exceptionResponse(fcode, ex);
                return;
            }
            if (field2 != 0xFF00 && field2 != 0x0000) { //Check value (status)
                exceptionResponse(fcode, EX_ILLEGAL_VALUE);
                return;
            }
            if (!Reg(COIL(field1), field2)) { //Check Address and execute (reg exists?)
                exceptionResponse(fcode, EX_ILLEGAL_ADDRESS);
                return;
            }
            if (Reg(COIL(field1)) != field2) { //Check for failure
                exceptionResponse(fcode, EX_SLAVE_FAILURE);
                return;
            }
            _reply = REPLY_ECHO;
            _onRequestSuccess(fcode, {COIL(field1), field2});
        break;

        case FC_WRITE_COILS:
            //field1 = startreg, field2 = numregs, frame[5] = bytecount, header len = 6
            ex = _onRequest(fcode, {COIL(field1), field2});
            if (ex != EX_SUCCESS) {
                exceptionResponse(fcode, ex);
                return;
            }
            bytecount_calc = field2 / 8;
            if (field2%8) bytecount_calc++;
            if (field2 < 0x0001 || field2 > MODBUS_MAX_BITS || 0xFFFF - field1 < field2 || frame[5] != bytecount_calc) { //Check registers range and data size maches
                exceptionResponse(fcode, EX_ILLEGAL_VALUE);
                return;
            }
            for (k = 0; k < field2; k++) { //Check Address (startreg...startreg + numregs)
                if (!searchRegister(COIL(field1) + k)) {
                    exceptionResponse(fcode, EX_ILLEGAL_ADDRESS);
                    return;
                }
            }
            if (!setMultipleBits(frame + 6, COIL(field1), field2)) {
                exceptionResponse(fcode, EX_SLAVE_FAILURE);
                return;
            }
            successResponce(COIL(field1), field2, fcode);
            _reply = REPLY_NORMAL;
            _onRequestSuccess(fcode, {COIL(field1), field2});
        break;
    #if defined(MODBUS_FILES)
        case FC_READ_FILE_REC:
            if (frame[1] < 0x07 || frame[1] > 0xF5) {   // Wrong request data size
                exceptionResponse(fcode, EX_ILLEGAL_VALUE);
                return;  
            }
            {
            uint8_t bufSize = 2;    // 2 bytes for frame header
            uint8_t* recs = frame + 2;   // Begin of sub-recs blocks
            uint8_t recsCount = frame[1] / 7; // Count of sub-rec blocks
            for (uint8_t p = 0; p < recsCount; p++) {   // Calc output buffer size required
                //uint16_t fileNum = (uint16_t)recs[1] << 8 | (uint16_t)recs[2];
                uint16_t recNum = (uint16_t)recs[3] << 8 | (uint16_t)recs[4];
                uint16_t recLen = (uint16_t)recs[5] << 8 | (uint16_t)recs[6];
                //Serial.printf("%d, %d, %d\n", fileNum, recNum, recLen);
                if (recs[0] != 0x06 || recNum > 0x270F) { // Wrong ref type or count of records
                    exceptionResponse(fcode, EX_ILLEGAL_ADDRESS);
                    return;
                }  
                bufSize += recLen * 2 + 2;   // 4 bytes for header + data
                recs += 7;
            }
            if (bufSize > MODBUS_MAX_FRAME) {  // Frame to return too large
                exceptionResponse(fcode, EX_ILLEGAL_ADDRESS);
                return;  
            }
            uint8_t* srcFrame = _frame;
            _frame = (uint8_t*)malloc(bufSize);
            if (!_frame) {
                free(srcFrame);
                exceptionResponse(fcode, EX_SLAVE_FAILURE);
                return;
            }
            _len = bufSize;
            recs = frame + 2;   // Begin of sub-recs blocks
            uint8_t* data = _frame + 2;
            for (uint8_t p = 0; p < recsCount; p++) {
                uint16_t fileNum = (uint16_t)recs[1] << 8 | (uint16_t)recs[2];
                uint16_t recNum = (uint16_t)recs[3] << 8 | (uint16_t)recs[4];
                uint16_t recLen = (uint16_t)recs[5] << 8 | (uint16_t)recs[6];
                ResultCode res = fileOp(fcode, fileNum, recNum, recLen, data + 2);
                if (res != EX_SUCCESS) {    // File read failed
                    free(srcFrame);
                    exceptionResponse(fcode, res);
                    return;  
                }
                data[0] = recLen * 2 + 1;
                data[1] = 0x06;
                data += recLen * 2 + 2;
                recs += 7;
            }
            _frame[0] = fcode;
            _frame[1] = bufSize;
            _reply = REPLY_NORMAL;
            free(srcFrame);
            }
        break;
        case FC_WRITE_FILE_REC: {
            if (frame[1] < 0x09 || frame[1] > 0xFB) {   // Wrong request data size
                exceptionResponse(fcode, EX_ILLEGAL_VALUE);
                return;  
            }
            uint8_t* recs = frame + 2;   // Begin of sub-recs blocks
            while (recs < frame + frame[1]) {
                if (recs[0] != 0x06) {
                    exceptionResponse(fcode, EX_ILLEGAL_ADDRESS);
                    return;  
                }
                uint16_t fileNum = (uint16_t)recs[1] << 8 | (uint16_t)recs[2];
                uint16_t recNum = (uint16_t)recs[3] << 8 | (uint16_t)recs[4];
                uint16_t recLen = (uint16_t)recs[5] << 8 | (uint16_t)recs[6];
                if (recs + recLen * 2 > frame + frame[1]) {
                    exceptionResponse(fcode, EX_ILLEGAL_ADDRESS);
                    return;
                }
                ResultCode res = fileOp(fcode, fileNum, recNum, recLen, recs + 7);
                if (res != EX_SUCCESS) {    // File write failed
                    exceptionResponse(fcode, res);
                    return;
                }
                recs += 7 + recLen * 2;
            }
        }
        _reply = REPLY_ECHO;
        break;
    #endif
        case FC_MASKWRITE_REG:
            //field1 = reg, field2 = AND mask, field3 = OR mask
            // Result = (Current Contents AND And_Mask) OR (Or_Mask AND (NOT And_Mask))
            field3 = (uint16_t)frame[5] << 8 | (uint16_t)frame[6];
            ex = _onRequest(fcode, {HREG(field1), field2, field3});
            if (ex != EX_SUCCESS) {
                exceptionResponse(fcode, ex);
                return;
            }
            field4 = Reg(HREG(field1));
            field4 = (field4 & field2) | (field3 & ~field2);
            if (!Reg(HREG(field1), field4)) { //Check Address and execute (reg exists?)
                exceptionResponse(fcode, EX_ILLEGAL_ADDRESS);
                return;
            }
            if (Reg(HREG(field1)) != field4) { //Check for failure
                exceptionResponse(fcode, EX_SLAVE_FAILURE);
                return;
            }
            _reply = REPLY_ECHO;
            _onRequestSuccess(fcode, {HREG(field1), field2, field3});
        break;
        case FC_READWRITE_REGS:
            //field1 = readreg, field2 = read count, frame[9] = data lenght, header len = 10
            //field3 = wtitereg, field4 = write count
            field3 = (uint16_t)frame[5] << 8 | (uint16_t)frame[6];
            field4 = (uint16_t)frame[7] << 8 | (uint16_t)frame[8];
            ex = _onRequest(fcode, {HREG(field1), field2, HREG(field3), field4});
            if (ex != EX_SUCCESS) {
                exceptionResponse(fcode, ex);
                return;
            }
            if (field2 < 0x0001 || field2 > MODBUS_MAX_WORDS ||
                field4 < 0x0001 || field4 > MODBUS_MAX_WORDS ||
                0xFFFF - field1 < field2 || 0xFFFF - field1 < field2 ||
                frame[9] != 2 * field4) { //Check value
                exceptionResponse(fcode, EX_ILLEGAL_VALUE);
                return;
            }
            if (!setMultipleWords((uint16_t*)(frame + 10), HREG(field3), field4)) {
                exceptionResponse(fcode, EX_SLAVE_FAILURE);
                return;
            }
            ex = readWords(HREG(field1), field2, fcode);
            if (ex != EX_SUCCESS) {
                exceptionResponse(fcode, ex);
                return;
            }
            _onRequestSuccess(fcode, {HREG(field1), field2, HREG(field3), field4});
        break;

        default:
            exceptionResponse(fcode, EX_ILLEGAL_FUNCTION);
            return;
    }
}

void Modbus::successResponce(TAddress startreg, uint16_t numoutputs, FunctionCode fn) {
    free(_frame);
	_len = 5;
    _frame = (uint8_t*) malloc(_len);
    if (!_frame) {
        _reply = REPLY_OFF;
	    return;
    }
    _frame[0] = fn;
    _frame[1] = startreg.address >> 8;
    _frame[2] = startreg.address & 0x00FF;
    _frame[3] = numoutputs >> 8;
    _frame[4] = numoutputs & 0x00FF;
}

void Modbus::exceptionResponse(FunctionCode fn, ResultCode excode) {
    free(_frame);
    _len = 2;
    _frame = (uint8_t*) malloc(_len);
    if (!_frame) {
        _reply = REPLY_OFF;
	    return;
    }
    _frame[0] = fn + 0x80;
    _frame[1] = excode;
    _reply = REPLY_NORMAL;
}

void Modbus::getMultipleBits(uint8_t* frame, TAddress startreg, uint16_t numregs) {
    uint8_t bitn = 0;
    uint16_t i = 0;
	while (numregs--) {
		if (BIT_BOOL(Reg(startreg)))
			bitSet(frame[i], bitn);
        else
			bitClear(frame[i], bitn);
		bitn++; //increment the bit index
		if (bitn == 8)  {
            i++;
            bitn = 0;
        }
		startreg++; //increment the register
	}
}

void Modbus::getMultipleWords(uint16_t* frame, TAddress startreg, uint16_t numregs) {
    for (uint8_t i = 0; i < numregs; i++) {
        frame[i] = __swap_16(Reg(startreg + i));
    }
}

Modbus::ResultCode Modbus::readBits(TAddress startreg, uint16_t numregs, FunctionCode fn) {
    if (numregs < 0x0001 || numregs > MODBUS_MAX_BITS || (0xFFFF - startreg.address) < numregs)
        return EX_ILLEGAL_VALUE;
    //Check Address
    //Check only startreg. Is this correct?
    //When I check all registers in range I got errors in ScadaBR
    //I think that ScadaBR request more than one in the single request
    //when you have more then one datapoint configured from same type.
#if defined(MODBUS_STRICT_REG)
    for (k = 0; k < numregs; k++) { //Check Address (startreg...startreg + numregs)
        if (!searchRegister(startreg + k))
            return EX_ILLEGAL_VALUE;
    }
#else
    if (!searchRegister(startreg))
        return EX_ILLEGAL_VALUE;
#endif
    free(_frame);
    //Determine the message length = function type, byte count and
	//for each group of 8 registers the message length increases by 1
	_len = 2 + numregs/8;
	if (numregs % 8) _len++; //Add 1 to the message length for the partial byte.
    _frame = (uint8_t*) malloc(_len);
    if (!_frame)
        return EX_SLAVE_FAILURE;
    _frame[0] = fn;
    _frame[1] = _len - 2; //byte count (_len - function code and byte count)
	_frame[_len - 1] = 0;  //Clean last probably partial byte
    getMultipleBits(_frame+2, startreg, numregs);
    _reply = REPLY_NORMAL;
    return EX_SUCCESS;
}

Modbus::ResultCode Modbus::readWords(TAddress startreg, uint16_t numregs, FunctionCode fn) {
    //Check value (numregs)
    if (numregs < 0x0001 || numregs > MODBUS_MAX_WORDS || 0xFFFF - startreg.address < numregs)
        return EX_ILLEGAL_VALUE;
#if defined(MODBUS_STRICT_REG)
    for (k = 0; k < numregs; k++) { //Check Address (startreg...startreg + numregs)
        if (!searchRegister(startreg + k))
            return EX_ILLEGAL_VALUE;
    }
#else
    if (!searchRegister(startreg))
        return EX_ILLEGAL_ADDRESS;
#endif
    free(_frame);
	_len = 2 + numregs * 2; //calculate the query reply message length. 2 bytes per register + 2 bytes for header
    _frame = (uint8_t*) malloc(_len);
    if (!_frame)
        return EX_SLAVE_FAILURE;
    _frame[0] = fn;
    _frame[1] = _len - 2;   //byte count
    getMultipleWords((uint16_t*)(_frame + 2), startreg, numregs);
    _reply = REPLY_NORMAL;
    return EX_SUCCESS;
}

bool Modbus::setMultipleBits(uint8_t* frame, TAddress startreg, uint16_t numoutputs) {
    uint8_t bitn = 0;
    uint16_t i = 0;
    bool result = true;
	while (numoutputs--) {
        Reg(startreg, BIT_VAL(bitRead(frame[i], bitn)));
        if (Reg(startreg) != BIT_VAL(bitRead(frame[i], bitn)))
            result = false;
        bitn++;     //increment the bit index
        if (bitn == 8) {
            i++;
            bitn = 0;
        }
        startreg++; //increment the register
	}
    return result;
}

bool Modbus::setMultipleWords(uint16_t* frame, TAddress startreg, uint16_t numregs) {
    bool result = true;
    for (uint8_t i = 0; i < numregs; i++) {
        Reg(startreg + i, __swap_16(frame[i]));
        if (Reg(startreg + i) != __swap_16(frame[i]))
            result = false;
    }
    return result;
}

bool Modbus::onGet(TAddress address, cbModbus cb, uint16_t numregs) {
	TRegister* reg;
	bool atLeastOne = false;
    if (!cb) {
        return removeOnGet(address);
    }
	while (numregs > 0) {
		reg = searchRegister(address);
		if (reg) {
            _callbacks.push_back({TCallback::ON_GET, address, cb});
			atLeastOne = true;
		}
		address++;
		numregs--;
	}
	return atLeastOne;
}
bool Modbus::onSet(TAddress address, cbModbus cb, uint16_t numregs) {
	TRegister* reg;
	bool atLeastOne = false;
    if (!cb) {
        return removeOnGet(address);
    }
	while (numregs > 0) {
		reg = searchRegister(address);
		if (reg) {
            _callbacks.push_back({TCallback::ON_SET, address, cb});
			atLeastOne = true;
		}
		address++;
		numregs--;
	}
	return atLeastOne;
}

bool Modbus::removeOn(TCallback::CallbackType t, TAddress address, cbModbus cb, uint16_t numregs) {
    size_t s = _callbacks.size();
    #if defined(MODBUS_USE_STL)
    #define MODBUS_COMPARE_ON [t, address, cb](const TCallback entry){\
        return entry.type == t && entry.address == address \
        && (!cb || std::addressof(cb) == std::addressof(entry.cb));}
    while(numregs--) {
        _callbacks.erase(remove_if(_callbacks.begin(), _callbacks.end(), MODBUS_COMPARE_ON), _callbacks.end());
        address++;
    }
    #else
    #define MODBUS_COMPARE_ON [t, address, cb](const TCallback entry){ \
        return entry.type == t && entry.address == address \
        && (!cb || entry.cb == cb);}
    while(numregs--) {
        size_t r = 0;
        do {
            r = _callbacks.find(MODBUS_COMPARE_ON);
            _callbacks.remove(r);
        } while (r < _callbacks.size());
        address++;
    }
    #endif
    return s == _callbacks.size();
}
bool Modbus::removeOnSet(TAddress address, cbModbus cb, uint16_t numregs) {
    return removeOn(TCallback::ON_SET, address, cb, numregs);
}

bool Modbus::removeOnGet(TAddress address, cbModbus cb, uint16_t numregs) {
    return removeOn(TCallback::ON_GET, address, cb, numregs);
}

bool Modbus::readSlave(uint16_t address, uint16_t numregs, FunctionCode fn) {
	free(_frame);
	_len = 5;
	_frame = (uint8_t*) malloc(_len);
    if (!_frame) {
        _reply = REPLY_OFF;
	    return false;
    }
	_frame[0] = fn;
	_frame[1] = address >> 8;
	_frame[2] = address & 0x00FF;
	_frame[3] = numregs >> 8;
	_frame[4] = numregs & 0x00FF;
	return true;
}

bool Modbus::writeSlaveBits(TAddress startreg, uint16_t to, uint16_t numregs, FunctionCode fn, bool* data) {
	free(_frame);
	_len = 6 + numregs/8;
	if (numregs % 8) _len++; //Add 1 to the message length for the partial byte.
    _frame = (uint8_t*) malloc(_len);
    if (!_frame) {
        _reply = REPLY_OFF;
	    return false;
    }
	_frame[0] = fn;
	_frame[1] = to >> 8;
	_frame[2] = to & 0x00FF;
	_frame[3] = numregs >> 8;
	_frame[4] = numregs & 0x00FF;
    _frame[5] = _len - 6;
    _frame[_len - 1] = 0;  //Clean last probably partial byte
    if (data) {
        boolToBits(_frame + 6, data, numregs);
    } else {
        getMultipleBits(_frame + 6, startreg, numregs);
    }
    _reply = REPLY_NORMAL;
    return true;
}

bool Modbus::writeSlaveWords(TAddress startreg, uint16_t to, uint16_t numregs, FunctionCode fn, uint16_t* data) {
	free(_frame);
	_len = 6 + 2 * numregs;
	_frame = (uint8_t*) malloc(_len);
    if (!_frame) {
        _reply = REPLY_OFF;
	    return false;    
    }
	_frame[0] = fn;
	_frame[1] = to >> 8;
	_frame[2] = to & 0x00FF;
	_frame[3] = numregs >> 8;
	_frame[4] = numregs & 0x00FF;
    _frame[5] = _len - 6;
    if (data) {
        uint16_t* frame = (uint16_t*)(_frame + 6);
        for (uint8_t i = 0; i < numregs; i++) {
            frame[i] = __swap_16(data[i]);
        }
    } else {
        getMultipleWords((uint16_t*)(_frame + 6), startreg, numregs);
    }
    return true;
}

void Modbus::boolToBits(uint8_t* dst, bool* src, uint16_t numregs) {
    uint8_t bitn = 0;
    uint16_t i = 0;
    uint16_t j = 0;
	while (numregs--) {
		if (src[j])
			bitSet(dst[i], bitn);
        else
			bitClear(dst[i], bitn);
		bitn++; //increment the bit index
		if (bitn == 8)  {
            i++;
            bitn = 0;
        }
		j++; //increment the register
	}
}

void Modbus::bitsToBool(bool* dst, uint8_t* src, uint16_t numregs) {
    uint8_t bitn = 0;
    uint16_t i = 0;
    uint16_t j = 0;
	while (numregs--) {
        dst[j] = bitRead(src[i], bitn);
        bitn++;     //increment the bit index
        if (bitn == 8) {
            i++;
            bitn = 0;
        }
        j++; //increment the register
	}
}

void Modbus::masterPDU(uint8_t* frame, uint8_t* sourceFrame, TAddress startreg, uint8_t* output) {
    uint8_t fcode  = frame[0];
    if ((fcode & 0x80) != 0) { // Check if error responce
	    _reply = frame[1];
	    return;
    }
    if (fcode != sourceFrame[0]) { // Check if responce matches the request
        _reply = EX_DATA_MISMACH;
        return;
    }
    _reply = EX_SUCCESS;
    uint16_t field2 = (uint16_t)sourceFrame[3] << 8 | (uint16_t)sourceFrame[4];
    uint8_t bytecount_calc;
    switch (fcode) {
        case FC_READ_REGS:
        case FC_READ_INPUT_REGS:
        case FC_READWRITE_REGS:
            //field2 = numregs, frame[1] = data lenght, header len = 2
            if (frame[1] != 2 * field2) { //Check if data size matches
                _reply = EX_DATA_MISMACH;
                break;
            }
            if (output) {
                uint16_t* from = (uint16_t*)(frame + 2);
                uint16_t* to = (uint16_t*)output;
                while(field2--) {
                    *(to++) = __swap_16(*(from++));
                }
            } else {
                setMultipleWords((uint16_t*)(frame + 2), startreg, field2);
            }
        break;
        case FC_READ_COILS:
        case FC_READ_INPUT_STAT:
            //field2 = numregs, frame[1] = data length, header len = 2
            bytecount_calc = field2 / 8;
            if (field2 % 8) bytecount_calc++;
            if (frame[1] != bytecount_calc) { // check if data size matches
                _reply = EX_DATA_MISMACH;
                break;
            }
            if (output) {
                bitsToBool((bool*)output, frame + 2, field2);
            } else {
                setMultipleBits(frame + 2, startreg, field2);
            }
        break;
    #if defined(MODBUS_FILES)
        case FC_READ_FILE_REC:
        // Should check if byte order swap needed
            if (frame[1] < 0x07 || frame[1] > 0xF5) {   // Wrong request data size
                _reply = EX_ILLEGAL_VALUE;
                return;  
            }
            {
            uint8_t* data = frame + 2;
            uint8_t* eoFrame = frame + frame[1];
            while (data < eoFrame) {
                //data[0] - sub-req length
                //data[1] = 0x06
                if (data[1] != 0x06 || data[0] < 0x07 || data[0] > 0xF5 || data + data[0] > eoFrame) {   // Wrong request data size
                    _reply = EX_ILLEGAL_VALUE;
                    return;  
                }
                memcpy(output, data + 2, data[0]);
                data += data[0] + 1;
                output += data[0] - 1;
            }
            }
        break;
        case FC_WRITE_FILE_REC:
    #endif
        case FC_WRITE_REG:
        case FC_WRITE_REGS:
        case FC_WRITE_COIL:
        case FC_WRITE_COILS:
        case FC_MASKWRITE_REG:
        break;

        default:
		    _reply = EX_GENERAL_FAILURE;
    }
}

void Modbus::cbEnable(bool state) {
    cbEnabled = state;
}
void Modbus::cbDisable() {
    cbEnable(false);
}
Modbus::~Modbus() {
    free(_frame);
}

#if defined(MODBUS_FILES)
#if defined(MODBUS_USE_STL)
bool Modbus::onFile(std::function<Modbus::ResultCode(Modbus::FunctionCode, uint16_t, uint16_t, uint16_t, uint8_t*)>  cb) {
#else
bool Modbus::onFile(Modbus::ResultCode (*cb)(Modbus::FunctionCode, uint16_t, uint16_t, uint16_t, uint8_t*)) {
#endif
    _onFile = cb;
    return true;
}
Modbus::ResultCode Modbus::fileOp(Modbus::FunctionCode fc, uint16_t fileNum, uint16_t recNum, uint16_t recLen, uint8_t* frame) {
    if (!_onFile) return EX_ILLEGAL_ADDRESS;
    return _onFile(fc, fileNum, recNum, recLen, frame);
}

    bool Modbus::readSlaveFile(uint16_t* fileNum, uint16_t* startRec, uint16_t* len, uint8_t count, FunctionCode fn) {
	    _len = count * 7 + 2;
        if (_len > MODBUS_MAX_FRAME) return false;
        free(_frame);
	    _frame = (uint8_t*) malloc(_len);
        if (!_frame) return false;
	    _frame[0] = fn;
	    _frame[1] = _len - 2;
        uint8_t* subReq = _frame + 2;
        for (uint8_t i = 0; i < count; i++) {
            subReq[0] = 0x06;
	        subReq[1] = fileNum[i] >> 8;
	        subReq[2] = fileNum[i] & 0x00FF;
            subReq[3] = startRec[i] >> 8;
	        subReq[4] = startRec[i] & 0x00FF;
            subReq[5] = len[i] >> 8;
	        subReq[6] = len[i] & 0x00FF;
            subReq += 7;
        }
        return true;
    }
    bool Modbus::writeSlaveFile(uint16_t* fileNum, uint16_t* startRec, uint16_t* len, uint8_t count, FunctionCode fn, uint8_t* data) {
        _len = 2;
        for (uint8_t i = 0; i < count; i++) {
            _len += len[i] * 2 + 7;
        }
        if (_len > MODBUS_MAX_FRAME) return false;
        free(_frame);
	    _frame = (uint8_t*) malloc(_len);
        if (!_frame) return false;
	    _frame[0] = fn;
	    _frame[1] = _len - 2;
        uint8_t* subReq = _frame + 2;
        for (uint8_t i = 0; i < count; i++) {
            subReq[0] = 0x06;
	        subReq[1] = fileNum[i] >> 8;
	        subReq[2] = fileNum[i] & 0x00FF;
            subReq[3] = startRec[i] >> 8;
	        subReq[4] = startRec[i] & 0x00FF;
            subReq[5] = len[i] >> 8;
	        subReq[6] = len[i] & 0x00FF;
            uint8_t clen = len[i] * 2;
            memcpy(subReq + 7, data, clen);
            subReq += 7 + clen;
            data += clen;
        }
        return true;
    }
    #endif

Modbus::ResultCode Modbus::_onRequestDefault(Modbus::FunctionCode fc, const RequestData data) {
    return EX_SUCCESS;
}
bool Modbus::onRequest(cbRequest cb) {
    _onRequest = cb;
    return true;
}
#if defined (MODBUSAPI_OPTIONAL)
bool Modbus::onRequestSuccess(cbRequest cb) {
    _onRequestSuccess = cb;
    return true;
}
#endif

#if defined(ARDUINO_SAM_DUE_STL)
namespace std {
    void __throw_bad_function_call() {
        Serial.println(F("STL ERROR - __throw_bad_function_call"));
        __builtin_unreachable();
    }
}
#endif