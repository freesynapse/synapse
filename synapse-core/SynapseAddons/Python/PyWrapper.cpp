
#include "pch.hpp"

#include "SynapseAddons/Python/PyWrapper.hpp"
#include "Synapse/Core.hpp"

namespace Syn
{

    int PyWrapper::s_npReturnCode = 1;

    //-----------------------------------------------------------------------------------
    bool PyWrapper::setPySymbolPtrs(const std::string& _module, 
                                    const std::string& _function, 
                                    CPyObject& _mod_ref, 
                                    CPyObject& _fnc_ref)
    {
        CPyObject pyModuleName = PyUnicode_FromString(_module.c_str());
        CPyObject mod = PyImport_Import(pyModuleName);
        PyErr_Print();
        if (mod)
        {
            _mod_ref = mod;

            CPyObject fnc = PyObject_GetAttrString(mod, _function.c_str());
            PyErr_Print();
            if (fnc && PyCallable_Check(fnc))
            {
                _fnc_ref = fnc;
                return true;
            }
            else
            {
                SYN_CORE_WARNING("Python function '", _function.c_str(), "' not found.");
                return false;
            }
        }
        else
        {
            SYN_CORE_WARNING("Python module '", _module.c_str(), "' not found.");
            return false;
        }

    }
    //-----------------------------------------------------------------------------------
    bool PyWrapper::setPyModulePtr(const std::string& _module, PyObject* _out_ptr)
    {
        CPyObject pyModuleName = PyUnicode_FromString(_module.c_str());
        _out_ptr = PyImport_Import(pyModuleName);
        printf("%s\n", __func__);
        if (_out_ptr)
            return true;
        else
        {
            SYN_CORE_WARNING("Python module '", _module.c_str(), "' not found.");
            return false;
        }
    }
    //-----------------------------------------------------------------------------------
    bool PyWrapper::setPyFunctionPtr(const std::string& _func, PyObject* _module_ptr, PyObject* _out_ptr)
    {
        printf("%s\n", __func__);
        _out_ptr = PyObject_GetAttrString(_module_ptr, _func.c_str());
        PyErr_Print();
        if (_out_ptr && PyCallable_Check(_out_ptr))
            return true;
        else
        {
            SYN_CORE_WARNING("Python function '", _func.c_str(), "' not found.");
            return false;
        }
    }
    //-----------------------------------------------------------------------------------
    #ifdef DEBUG_PYTHON_EMBEDDING
    void PyWrapper::run_np_func(const std::string& _src, const std::string& _func)
    {
        CPyObject pyName = PyUnicode_FromString(_src.c_str());
        CPyObject pyModule = PyImport_Import(pyName);
        // repopulate function pointer tables in NumPy
        _import_array();

        // build 'numpy' array
        const int size = 300;
        printf("c++: dimensions = %d\n", size*size);
        const int nd = 2;
        npy_intp dims[nd] = { size , size };
        long double (*c_arr)[size] = { new long double[size][size] };
        for (int i = 0; i < size; i++)
            for (int j = 0; j < size; j++)
                c_arr[i][j] = i + j;
        //const int nd = 1;
        //npy_intp dims[nd] = { size };
        //long double* c_arr = new long double[size];
        //for (int i = 0; i < size; i++)
        //    c_arr[i] = i;

        PyObject* pyArray = PyArray_SimpleNewFromData(nd, dims, NPY_LONGDOUBLE, (void*)c_arr);

        // get function pointer
        if (pyModule)
        {
            CPyObject pyFunc = PyObject_GetAttrString(pyModule, _func.c_str());
            
            printf("module   : 0x%" PRIXPTR "\n", (uintptr_t)pyModule);
            printf("function : 0x%" PRIXPTR "\n", (uintptr_t)pyFunc);
            
            if (pyFunc && PyCallable_Check(pyFunc))
            {
                PyObject* pyReturn = PyObject_CallFunctionObjArgs(pyFunc, pyArray, pyArray, NULL);
                PyArrayObject* np_ret = reinterpret_cast<PyArrayObject*>(pyReturn);

                // Convert back to c++ and print
                printf("From c++\n");
                int len = PyArray_SHAPE(np_ret)[0];
                printf("pyResult len = %d\n", len);
                long double* c_out = reinterpret_cast<long double*>(PyArray_DATA(np_ret));
                int ndots = 3;
                int cdots = 0;
                for (int i = 0; i < len; i++)
                {
                    if ((i < 3) || (i > len - 3))
                        printf("%d: %Lf\n", i, c_out[i]);
                    else if (cdots < ndots)
                    {
                        printf(" ...\n");
                        cdots++;
                    }
                }

            }
            else
            {
                SYN_CORE_WARNING("Function '", _func.c_str(), "' not found (in '", _src.c_str(), "').");
            }
        }
        else
        {
            SYN_CORE_WARNING("Failed to import Python module '", _src.c_str(), ".py',");
        }
    }
    #endif
    
}
