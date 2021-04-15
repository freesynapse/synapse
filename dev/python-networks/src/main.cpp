
#include <iostream>
#include <vector>
#include <fstream>


//---------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    // read file (binary)
    std::ifstream fin;
    fin.open("/home/iomanip/documents/_BIOINFO--ML/_EXP/graph-theory/networks/jagmesh8.network", std::ios::in|std::ios::binary);

    int nnodes, nedges;
    fin.read((char*)&nnodes, sizeof(int));
    fin.read((char*)&nedges, sizeof(int));

    std::vector<float> nodes(nnodes);
    std::vector<float> edges(nedges);

    std::cout << "nnodes=" << nnodes << ", nedges=" << nedges << "\n";
    std::cout << "v size=" << nodes.size() << '\n';

    fin.read((char*)&(nodes[0]), sizeof(float)*nodes.size());
    fin.read((char*)&(edges[0]), sizeof(float)*edges.size());

    std::cout << "10 nodes:\n";
    for (size_t i = 0; i < 10; i++)
        std::cout << i << ": [" << nodes[i*2+0] << ", " << nodes[i*2+1] << "]\n";
    std::cout << "10 edges:\n";
    for (size_t i = 0; i < 10; i++)
    {
        std::cout << i << ": [" << nodes[i*4+0] << ", " << nodes[i*4+1] << "], ";
        std::cout << "[" << nodes[i*4+2] << ", " << nodes[i*4+3] << "]\n";
    }
    
    std::cout << "last node: [" << nodes[nnodes-2] << ", " << nodes[nnodes-1] << "]\n";
    fin.close();


    return 0;
}



