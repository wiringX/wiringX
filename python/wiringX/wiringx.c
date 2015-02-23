/*
	Copyright (c) 2014 CurlyMo <curlymoo1@gmail.com>

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <poll.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <time.h>
#include <sys/wait.h>
#include <pthread.h>

#include "Python.h"

#include "wiringX.h"

PyThreadState *mainThreadState;
static PyObject *module = NULL;

typedef struct threadparams_t {
	int gpio;
	int time;
	PyObject *callback;
} threadparams_t;

static void py_wiringXLog(int prio, const char *format_str, ...) {
	if(prio == LOG_ERR) {
		char line[1024];
		va_list ap;
		va_start(ap, format_str);
		vsprintf(line, format_str, ap);
		PyErr_SetString(PyExc_Exception, line); 
		va_end(ap);
	}
}

static PyObject *py_digitalWrite(PyObject *self, PyObject *args) {
	int gpio = 0, value = 0;

	if(!PyArg_ParseTuple(args, "ii", &gpio, &value)) {
		return NULL;
	}

	digitalWrite(gpio, value);

	Py_RETURN_NONE;
}

static PyObject *py_digitalRead(PyObject *self, PyObject *args) {
	int gpio = 0, ret = 0;

	if(!PyArg_ParseTuple(args, "i", &gpio)) {
		return NULL;
	}

	ret = digitalRead(gpio);
	if(ret < 0) {
		return PyErr_SetFromErrno(PyExc_IOError);
	}

	return Py_BuildValue("i", ret);
}

static PyObject *py_pinMode(PyObject *self, PyObject *args) {
	int gpio = 0, value = 0;

	if(!PyArg_ParseTuple(args, "ii", &gpio, &value)) {
		return NULL;
	}

	pinMode(gpio, value);

	Py_RETURN_NONE;
}

/*
static PyObject *py_wiringXISR(PyObject *self, PyObject *args) {
	int gpio = 0, edge = 0;

	if(!PyArg_ParseTuple(args, "ii", &gpio, &edge)) {
		return NULL;
	}

	wiringXISR(gpio, edge);

	Py_RETURN_NONE;
}

void *interrupt(void *param){
	PyObject *arglist;
	struct threadparams_t *p = (struct threadparams_t *)param;
	PyObject *callback = malloc(sizeof(PyObject)), *result = NULL;
	PyGILState_STATE state;
	memcpy(&callback, &p->callback, sizeof(PyObject));
	int gpio = p->gpio, time = p->time, x = 0;

	while(1) {
		state = PyGILState_Ensure();
		arglist = Py_BuildValue("(i)", x);
		result = PyEval_CallObject(callback, arglist);
		Py_DECREF(arglist);
		if(result == NULL) {
			return NULL;
		}
		Py_DECREF(result);
		PyGILState_Release(state);
		x = waitForInterrupt(gpio, time);
	}
	return NULL;
}

static PyObject *py_waitForInterrupt(PyObject *self, PyObject *args) {
	struct threadparams_t p;
	pthread_t tid;

	if(!PyArg_ParseTuple(args, "Oii", &p.callback, &p.gpio, &p.time)) {
		return NULL;
	}

	if(!PyCallable_Check(p.callback)) {
		PyErr_SetString(PyExc_ValueError, "first argument should be callable");
		return NULL;
	}

	PyEval_InitThreads();

	if(pthread_create(&tid, NULL, interrupt, (void *)&p) < 0) {
		return PyErr_SetFromErrno(PyExc_RuntimeError);
	}
	sleep(1);
	if(pthread_detach(tid) < 0) {
		return PyErr_SetFromErrno(PyExc_RuntimeError);
	}
	
	return PyInt_FromLong(tid);
}
*/

static PyObject *py_validGPIO(PyObject *self, PyObject *args) {
	int gpio = 0;

	if(!PyArg_ParseTuple(args, "i", &gpio)) {
		return NULL;
	}

	if(wiringXValidGPIO(gpio) == 0) {
		return Py_True;
	} else {
		return Py_False;
	}
}

static PyObject *py_gc(PyObject *self, PyObject *noarg) {
	wiringXGC();
	Py_RETURN_NONE;
}

static PyObject *py_platform(PyObject *self, PyObject *noarg) {
	return Py_BuildValue("s", wiringXPlatform());
}

static PyObject *py_I2CRead(PyObject *self, PyObject *args) {
	int fd = 0;

	if(!PyArg_ParseTuple(args, "i", &fd)) {
		return NULL;
	}

	return Py_BuildValue("i", wiringXI2CRead(fd));
}

static PyObject *py_I2CReadReg8(PyObject *self, PyObject *args) {
	int fd = 0, reg = 0;

	if(!PyArg_ParseTuple(args, "ii", &fd, &reg)) {
		return NULL;
	}

	return Py_BuildValue("i", wiringXI2CReadReg8(fd, reg));
}

static PyObject *py_I2CReadReg16(PyObject *self, PyObject *args) {
	int fd = 0, reg = 0;

	if(!PyArg_ParseTuple(args, "ii", &fd, &reg)) {
		return NULL;
	}

	return Py_BuildValue("i", wiringXI2CReadReg16(fd, reg));
}

static PyObject *py_I2CWrite(PyObject *self, PyObject *args) {
	int fd = 0, data = 0;

	if(!PyArg_ParseTuple(args, "ii", &fd, &data)) {
		return NULL;
	}

	if(Py_BuildValue("i", wiringXI2CWrite(fd, data)) == 0) {
		return Py_True;
	} else {
		return Py_False;
	}
}

static PyObject *py_I2CWriteReg8(PyObject *self, PyObject *args) {
	int fd = 0, data = 0, reg = 0;

	if(!PyArg_ParseTuple(args, "iii", &fd, &reg, &data)) {
		return NULL;
	}

	if(Py_BuildValue("i", wiringXI2CWriteReg8(fd, reg, data)) == 0) {
		return Py_True;
	} else {
		return Py_False;
	}
}

static PyObject *py_I2CWriteReg16(PyObject *self, PyObject *args) {
	int fd = 0, data = 0, reg = 0;

	if(!PyArg_ParseTuple(args, "iii", &fd, &reg, &data)) {
		return NULL;
	}

	if(Py_BuildValue("i", wiringXI2CWriteReg16(fd, reg, data)) == 0) {
		return Py_True;
	} else {
		return Py_False;
	}
}

static PyObject *py_setupI2C(PyObject *self, PyObject *args) {
	int device = 0;

	if(!PyArg_ParseTuple(args, "i", &device)) {
		return NULL;
	}

	return Py_BuildValue("i", wiringXI2CSetup(device));
}

static PyObject *py_SPIGetFd(PyObject *self, PyObject *args) {
	int channel = 0;

	if(!PyArg_ParseTuple(args, "i", &channel)) {
		return NULL;
	}

	return Py_BuildValue("i", wiringXSPIGetFd(channel));
}

static PyObject *py_SPIDataRW(PyObject *self, PyObject *args) {
	int channel = 0, len = 0;
	Py_buffer data;
	if(!PyArg_ParseTuple(args, "is*i", &channel, &data, &len)) {
		return NULL;
	}

	int result = wiringXSPIDataRW(channel, (unsigned char *)data.buf, len);

	if(result < 0) {
		return NULL;
	}

	PyObject *out = Py_BuildValue("s", data.buf);
	PyBuffer_Release(&data);
	return out;
}

static PyObject *py_setupSPI(PyObject *self, PyObject *args) {
	int channel = 0, speed = 0;

	if(!PyArg_ParseTuple(args, "ii", &channel, &speed)) {
		return NULL;
	}

	return Py_BuildValue("i", wiringXSPISetup(channel, speed));
}

static PyObject *py_setup(PyObject *self, PyObject *noarg) {
	char name[8];
	int pin = 0;

	if(wiringXSetup() < 0){
		return PyErr_SetFromErrno(PyExc_IOError);
	}

	while(wiringXValidGPIO(pin) == 0) {
		sprintf(name, "pin%d", pin);
		PyModule_AddObject(module, name, Py_BuildValue("i", pin));

		sprintf(name, "PIN%d", pin);
		PyModule_AddObject(module, name, Py_BuildValue("i", pin));

		pin++;
	}
	Py_RETURN_NONE;
}

/* Define module methods */
static PyMethodDef module_methods[] = {
    {"setup", py_setup, METH_NOARGS, "Initialize module"},
    {"pinMode", py_pinMode, METH_VARARGS,	"Set pin mode"},
    {"digitalWrite", py_digitalWrite, METH_VARARGS,	"Set output state"},
    {"digitalRead", py_digitalRead, METH_VARARGS,	"Read input state"},
    {"valid", py_validGPIO, METH_VARARGS,	"Check if GPIO is valid"},
    {"gc", py_gc, METH_NOARGS, "Garbage collect wiringX"},
    {"platform", py_platform, METH_NOARGS, "Get platform we're running on"},
    {"I2CSetup", py_setupI2C, METH_VARARGS, "Setup I2C device"},
    {"I2CRead", py_I2CRead, METH_VARARGS, "Read from I2C device"},
    {"I2CReadReg8", py_I2CReadReg8, METH_VARARGS, "Read from I2C device"},
    {"I2CReadReg16", py_I2CReadReg16, METH_VARARGS, "Read from I2C device"},
    {"I2CWrite", py_I2CWrite, METH_VARARGS, "Write to I2C device"},
    {"I2CWriteReg8", py_I2CWriteReg8, METH_VARARGS, "Write to I2C device"},
    {"I2CWriteReg16", py_I2CWriteReg16, METH_VARARGS, "Write to I2C device"},
    {"SPIGetFd", py_SPIGetFd, METH_VARARGS, "Get SPI file descriptor for channel"},
    {"SPIDataRW", py_SPIDataRW, METH_VARARGS, "Read / write SPI device"},
    {"SPISetup", py_setupSPI, METH_VARARGS, "Setup SPI device"},
    /*{"ISR", py_wiringXISR, METH_VARARGS,	"Set pin to interrupt"},
    {"waitForInterrupt", py_waitForInterrupt, METH_VARARGS,	"Wait for interrupt"},*/
		
    {NULL, NULL, 0, NULL}
};

#if PY_MAJOR_VERSION >= 3
static struct PyModuleDef module_def = {
    PyModuleDef_HEAD_INIT,
    "gpio",
    NULL,
    -1,
    module_methods
};
#endif

#if PY_MAJOR_VERSION >= 3
PyMODINIT_FUNC PyInit_gpio(void) {
	module = PyModule_Create(&module_def);
#else
PyMODINIT_FUNC initgpio(void) {
	module = Py_InitModule("gpio", module_methods);
#endif

	if(module == NULL) {
#if PY_MAJOR_VERSION >= 3
		return NULL;
#else
		return;
#endif
	}

	wiringXLog = py_wiringXLog;
	
	PyModule_AddObject(module, "HIGH", Py_BuildValue("i", 1));
	PyModule_AddObject(module, "LOW", Py_BuildValue("i", 0));
	PyModule_AddObject(module, "INPUT", Py_BuildValue("i", INPUT));
	PyModule_AddObject(module, "OUTPUT", Py_BuildValue("i", OUTPUT));
	PyModule_AddObject(module, "INT_EDGE_BOTH", Py_BuildValue("i", INT_EDGE_BOTH));
	PyModule_AddObject(module, "INT_EDGE_RISING", Py_BuildValue("i", INT_EDGE_RISING));
	PyModule_AddObject(module, "INT_EDGE_FALLING", Py_BuildValue("i", INT_EDGE_FALLING));

#if PY_MAJOR_VERSION >= 3
	return module;
#else
	return;
#endif
}
