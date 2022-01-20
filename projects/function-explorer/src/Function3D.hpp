
#pragma once

#include <Synapse/Types/Meshgrid.hpp>
#include <Synapse/Renderer/Renderer.hpp>
#include <Synapse/Renderer/Transform.hpp>
#include <Synapse/Renderer/Mesh/MeshShape.hpp>
#include <Synapse/Core.hpp>
#include <Synapse/Renderer/Buffers/VertexArray.hpp>
#include <muparser/muParser.h>


#define DEBUG_FUNCTION_3D
#undef DEBUG_FUNCTION_3D


namespace Syn
{
    typedef struct function_vertex_t
    {
        glm::vec3 position; // 12 bytes
        glm::vec3 normal;   // 12 bytes
    } function_vertex_t;    // 24 bytes

    //
    class Function3D : public Meshgrid<float>
    {
    private:
        // function evaluation result
        float* m_z = nullptr;
        uint32_t m_size = 0;    // number of z
        
        // mu parser handles
        std::string m_parserExpression = "";        
        mu::Parser m_parser;
        double m_parser_x = 0.0;
        double m_parser_y = 0.0;
        double m_parser_t = 0.0;

        // rendering variables
        uint32_t m_normalFlag = MESH_NORMALS_APPROX_FAST;
        Ref<MeshShape> m_mesh;


        
    public:
        // ctor
        Function3D()
        {
            m_parser.DefineVar("x", &m_parser_x);
            m_parser.DefineVar("y", &m_parser_y);
        }
        Function3D(const Linspace<float>& _x, const Linspace<float>& _y)
        {
            m_x = _x;
            m_y = _y;

            m_parser.DefineVar("x", &m_parser_x);
            m_parser.DefineVar("y", &m_parser_y);
            m_parser.DefineVar("t", &m_parser_t);
        }

        // dtor -- free memory
        ~Function3D()
        {
            if (m_z != nullptr)
                delete[] m_z;
        }

        // accessors
        const float* getZValues() const { return m_z; }
        const uint32_t getZCount() const { return m_size; }
        void setNormalCalcMethod(uint32_t _flag=MESH_NORMALS_APPROX_FAST) { m_normalFlag = _flag; }
        const Ref<MeshShape>& getMesh() const { return m_mesh; }
        const std::string& getExpr() const { return m_parserExpression; }
        
        // update linspaces
        void updateXAxis(const Linspace<float>& _x) { m_x = _x;  evaluate(); }
        void updateYAxis(const Linspace<float>& _y) { m_y = _y;  evaluate(); }
        void updateAxes(const Linspace<float>& _x, const Linspace<float>& _y) { m_x = _x; m_y = _y; evaluate(); }

        /* Renders the geometry. */
        void render(const Ref<Shader>& _shader) const { m_mesh->render(_shader); }

        /*
         * Update the values, using the Linspaces m_x, m_y and the
         * expression set (stored in m_parserExpression). Uses the 
         * muParser library for function evaluation. Recalculates 
         * mesh and uploads to GPU.
         */
        void evaluateExpression(const std::string& _expr)
        {
            m_parserExpression = _expr;
            m_parser.SetExpr(m_parserExpression);
            evaluate();
        }
        void evaluateExpression(const std::string& _expr, 
                                const Linspace<float>& _x, 
                                const Linspace<float>& _y)
        {
            m_parserExpression = _expr;
            m_parser.SetExpr(m_parserExpression);
            m_x = _x;
            m_y = _y;
            evaluate();
        }
        
    private:
        void evaluate()
        {
            SYN_PROFILE_FUNCTION();

            Timer timer("", false);

            //
            if (m_parserExpression == "")
                return;

            // is this an update?
            if (m_z != nullptr)
            {
                #ifdef DEBUG_FUNCTION_3D
                    SYN_CORE_TRACE("deleting m_z ptr.");
                #endif
                delete[] m_z;
                m_size = 0;
            }

            // memory allocation
            int nx = m_x.size();
            int ny = m_y.size();
            int nz = nx * ny;

            m_z = new float[nz];
            m_size = nz;
            #ifdef DEBUG_FUNCTION_3D
                SYN_CORE_TRACE("Allocated ", m_size, " values (", sizeof(float) * m_size, " bytes).")
                SYN_CORE_TRACE("Allocated ", m_size, " normals (", sizeof(float) * m_size, " bytes).")
            #endif

            float* x_vals = m_x.getValues();
            float* y_vals = m_y.getValues();
            for (int i = 0; i < ny; i++)
            {
                for (int j = 0; j < nx; j++)
                {
                    // index into 1d array
                    uint32_t index = i * nx + j;
                    // update parser values
                    m_parser_x = (double)x_vals[j];
                    m_parser_y = (double)y_vals[i];
                    m_z[index] = m_parser.Eval();
                }
            }

            // get mesh
            m_mesh = MeshCreator::createShapeMeshgrid(m_z, m_size, m_x, m_y,
                MESH_ATTRIB_POSITION | MESH_ATTRIB_NORMAL, m_normalFlag);


            #ifdef DEBUG_FUNCTION_3D
                SYN_CORE_TRACE("function evaluated in ", timer.getDeltaTimeMs(), " ms.");
            #endif

        }

    };


}



