
#pragma once

#include "Synapse/Core.hpp"
#include "Synapse/Utils/Timer/Timer.hpp"
#include "Synapse/Debug/Profiler.hpp"

#define PY_SSIZE_T_CLEAN
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION

#include <Python.h>
#include <numpy/arrayobject.h>


namespace Syn
{

    //
    class CPyObject
    {
    private:
        PyObject* m_pyPtr;

    public:
        CPyObject() :
            m_pyPtr(nullptr)
        {}

        CPyObject(PyObject* _ptr) :
            m_pyPtr(_ptr)
        {}

        ~CPyObject()
        {
            if (m_pyPtr)
                Py_DECREF(m_pyPtr);
            m_pyPtr = nullptr;
        }

        PyObject* addRef()
        {
            if (m_pyPtr)
                Py_INCREF(m_pyPtr);
            return m_pyPtr;
        }

        PyObject* set(PyObject* _ptr)       { return (m_pyPtr = _ptr); }
        PyObject* get()               const { return m_pyPtr;          }

        bool is() { return m_pyPtr ? true : false; }

        PyObject* operator->() { return m_pyPtr; }
        operator PyObject*() { return m_pyPtr; }
        PyObject* operator=(PyObject* _ptr) { m_pyPtr = _ptr; return m_pyPtr; }
        operator bool() { return m_pyPtr ? true : false; }

    };

    //
    class PyWrapper
    {
    private:
        static int s_npReturnCode;    // ugly hack to debug import_array()

    public:
        static void init(const std::string& _script_path="../assets/python_scripts/")
        { 
            SYN_PROFILE_FUNCTION();
            Timer timer("", false);

            /* Initialize the python interpreter */
            Py_Initialize();

            /* 
            * Append to python sys path instead of setting the env 
            * path, so that module imports in loadable python scripts 
            * works (i.e. not messing with the python path).
            */
            PyRun_SimpleString("import sys");
            std::string path = "sys.path.append('" + _script_path + "')";
            PyRun_SimpleString(path.c_str());

            int* ret = init_numpy();
            if (ret == NULL)
            {
                SYN_CORE_WARNING("NumPy function pointer tables not initialized.");
            }
            else
            {
                SYN_CORE_TRACE("NumPy function pointer tables initalized.")
            }

            SYN_CORE_TRACE("Python interpreter initialized in ", timer.getDeltaTimeMs(), " ms.");
        }

        /* Helper function for getting pointers to python symbols. 
         * Built-in error handling; returns true on success, false 
         * on failure. 
         */
        static bool setPySymbolPtrs(const std::string& _module, 
                                    const std::string& _function, 
                                    CPyObject& _mod_ref, 
                                    CPyObject& _fnc_ref);

        /* Helper function for getting pointers to python module. 
         * Built-in error handling; returns true on success, false 
         * on failure. 
         */
        static bool setPyModulePtr(const std::string& _module, PyObject* _out_ptr);
        
        /* Helper function for getting pointers to python function. 
         * Built-in error handling; returns true on success, false 
         * on failure. 
         */
        static bool setPyFunctionPtr(const std::string& _func, PyObject* _module_ptr, PyObject* _out_ptr);

        #ifdef DEBUG_PYTHON_EMBEDDING
            /* Function taking a function string as argument and storing the result in a 2d array */
            static void run_np_func(const std::string& _src, const std::string& _func);
        #endif

        /* Shut down python interpreter */
        static void shutdown() 
        {
            SYN_PROFILE_FUNCTION();
            Timer timer("", false);

            Py_Finalize(); 
            SYN_CORE_TRACE("Python interpreter terminated (", timer.getDeltaTimeMs(), " ms).");
        }

    private:
        static int* init_numpy()
        {
            /*
             * The import_array() function (actually a #define wrapper around the _import_array() 
             * function) needs to be called before using arrays in the NumPy API. Initializes function 
             * pointer tables.
             * N.B.: return NULL on failure, so init() needs to be of type int*.
             */
            PyRun_SimpleString("import numpy as np");
            import_array();
            
            return &s_npReturnCode;
        }

    };

        
}
