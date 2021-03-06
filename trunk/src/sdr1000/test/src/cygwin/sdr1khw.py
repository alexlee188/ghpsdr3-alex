# This file was created automatically by SWIG 1.3.29.
# Don't modify this file, modify the SWIG interface instead.
# This file is compatible with both classic and new-style classes.

import _sdr1khw
import new
new_instancemethod = new.instancemethod
def _swig_setattr_nondynamic(self,class_type,name,value,static=1):
    if (name == "thisown"): return self.this.own(value)
    if (name == "this"):
        if type(value).__name__ == 'PySwigObject':
            self.__dict__[name] = value
            return
    method = class_type.__swig_setmethods__.get(name,None)
    if method: return method(self,value)
    if (not static) or hasattr(self,name):
        self.__dict__[name] = value
    else:
        raise AttributeError("You cannot add attributes to %s" % self)

def _swig_setattr(self,class_type,name,value):
    return _swig_setattr_nondynamic(self,class_type,name,value,0)

def _swig_getattr(self,class_type,name):
    if (name == "thisown"): return self.this.own()
    method = class_type.__swig_getmethods__.get(name,None)
    if method: return method(self)
    raise AttributeError,name

def _swig_repr(self):
    try: strthis = "proxy of " + self.this.__repr__()
    except: strthis = ""
    return "<%s.%s; %s >" % (self.__class__.__module__, self.__class__.__name__, strthis,)

import types
try:
    _object = types.ObjectType
    _newclass = 1
except AttributeError:
    class _object : pass
    _newclass = 0
del types


PIO_IC01 = _sdr1khw.PIO_IC01
PIO_IC03 = _sdr1khw.PIO_IC03
PIO_IC08 = _sdr1khw.PIO_IC08
PIO_IC11 = _sdr1khw.PIO_IC11
PIO_NONE = _sdr1khw.PIO_NONE
RFE_IC07 = _sdr1khw.RFE_IC07
RFE_IC09 = _sdr1khw.RFE_IC09
RFE_IC10 = _sdr1khw.RFE_IC10
RFE_IC11 = _sdr1khw.RFE_IC11
SER = _sdr1khw.SER
SCK = _sdr1khw.SCK
SCLR_NOT = _sdr1khw.SCLR_NOT
DCDR_NE = _sdr1khw.DCDR_NE
DDSWRB = _sdr1khw.DDSWRB
DDSRESET = _sdr1khw.DDSRESET
COMP_PD = _sdr1khw.COMP_PD
BYPASS_PLL = _sdr1khw.BYPASS_PLL
BYPASS_SINC = _sdr1khw.BYPASS_SINC
class SDR1000(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, SDR1000, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, SDR1000, name)
    __repr__ = _swig_repr
    def __init__(self, *args): 
        this = _sdr1khw.new_SDR1000(*args)
        try: self.this.append(this)
        except: self.this = this
    __swig_destroy__ = _sdr1khw.delete_SDR1000
    __del__ = lambda self : None;
    def Latch(*args): return _sdr1khw.SDR1000_Latch(*args)
    def SRLoad(*args): return _sdr1khw.SDR1000_SRLoad(*args)
    def WriteDDS(*args): return _sdr1khw.SDR1000_WriteDDS(*args)
    def ResetDDS(*args): return _sdr1khw.SDR1000_ResetDDS(*args)
    __swig_setmethods__["sdr1kusb"] = _sdr1khw.SDR1000_sdr1kusb_set
    __swig_getmethods__["sdr1kusb"] = _sdr1khw.SDR1000_sdr1kusb_get
    if _newclass:sdr1kusb = property(_sdr1khw.SDR1000_sdr1kusb_get, _sdr1khw.SDR1000_sdr1kusb_set)
    __swig_setmethods__["usb"] = _sdr1khw.SDR1000_usb_set
    __swig_getmethods__["usb"] = _sdr1khw.SDR1000_usb_get
    if _newclass:usb = property(_sdr1khw.SDR1000_usb_get, _sdr1khw.SDR1000_usb_set)
    def StandBy(*args): return _sdr1khw.SDR1000_StandBy(*args)
    def PowerOn(*args): return _sdr1khw.SDR1000_PowerOn(*args)
    def StatusPort(*args): return _sdr1khw.SDR1000_StatusPort(*args)
    def UpdateHW(*args): return _sdr1khw.SDR1000_UpdateHW(*args)
    def SetFreq(*args): return _sdr1khw.SDR1000_SetFreq(*args)
    def SetBPF(*args): return _sdr1khw.SDR1000_SetBPF(*args)
    def SetLPF(*args): return _sdr1khw.SDR1000_SetLPF(*args)
    def SetPALPF(*args): return _sdr1khw.SDR1000_SetPALPF(*args)
    def SetMute(*args): return _sdr1khw.SDR1000_SetMute(*args)
    def SetINAOn(*args): return _sdr1khw.SDR1000_SetINAOn(*args)
    def SetATTOn(*args): return _sdr1khw.SDR1000_SetATTOn(*args)
    def SetTRX_TR(*args): return _sdr1khw.SDR1000_SetTRX_TR(*args)
    def SetRFE_TR(*args): return _sdr1khw.SDR1000_SetRFE_TR(*args)
    def SetPA_TR(*args): return _sdr1khw.SDR1000_SetPA_TR(*args)
    def SetXVTR_TR(*args): return _sdr1khw.SDR1000_SetXVTR_TR(*args)
    def SetXVTR_RF(*args): return _sdr1khw.SDR1000_SetXVTR_RF(*args)
    def SetX2(*args): return _sdr1khw.SDR1000_SetX2(*args)
    def SetImpOn(*args): return _sdr1khw.SDR1000_SetImpOn(*args)
    def SetPA_Bias(*args): return _sdr1khw.SDR1000_SetPA_Bias(*args)
    def SetClockRefFreq(*args): return _sdr1khw.SDR1000_SetClockRefFreq(*args)
    def SetFreqCalOffset(*args): return _sdr1khw.SDR1000_SetFreqCalOffset(*args)
    def SetSpurReductionMask(*args): return _sdr1khw.SDR1000_SetSpurReductionMask(*args)
    def DoImpulse(*args): return _sdr1khw.SDR1000_DoImpulse(*args)
    def PA_ReadADC(*args): return _sdr1khw.SDR1000_PA_ReadADC(*args)
    def ATU_Tune(*args): return _sdr1khw.SDR1000_ATU_Tune(*args)
    def ReadDDSReg(*args): return _sdr1khw.SDR1000_ReadDDSReg(*args)
    def WriteDDSReg(*args): return _sdr1khw.SDR1000_WriteDDSReg(*args)
SDR1000_swigregister = _sdr1khw.SDR1000_swigregister
SDR1000_swigregister(SDR1000)
cvar = _sdr1khw.cvar



