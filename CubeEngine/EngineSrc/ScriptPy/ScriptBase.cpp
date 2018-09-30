#include "ScriptBase.h"
#include "Base/Log.h"
#ifdef _DEBUG
#undef _DEBUG
#include <Python27/include/Python.h>
#define _DEBUG
#else
#include <Python.h>
#endif
namespace tzw
{
	namespace ScriptBase
	{
		static PyObject* py_log(PyObject* self, PyObject* args)
		{
			const char* msg = NULL;
			if (!PyArg_ParseTuple(args, "s", &msg))
			{
				return 0;
			}
			tlog("%s\n", msg);
			Py_RETURN_NONE;
		}
		static PyMethodDef module_methods[] = {
			{ "log", py_log, METH_VARARGS, "" },
			{ NULL }
		};
		void initModules()
		{
			PyObject *module = Py_InitModule3("tlib", module_methods, "tzw module");
			if (PyErr_Occurred()) { PyErr_Print(); PyErr_Clear(); return ; }
		}
	}
}