
#include <iostream>
#include <string>
#include <fstream>


int main(int argc, char* argv[])
{
    // find the viewport specified in the apps ImGui.ini (if present)
    std::ifstream fs("./imgui.ini", std::ios::in);
    if (!fs)
    {
        std::cout << "No ImGui configuration file found.\n";
        return 0;
    }

    // read the preset viewport for the renderer in config file
    // under 
    // [Window][synapse-core::renderer]
    // Pos=0,22
    // Size=1528,1031
    // Collapsed=0
    // DockId=0x00000001,0
    //
    // Both Pos and Size are needed:
    // the viewport size = (size.x - pos.x, size.y - pos.y).
    //
    std::string imgui_id = "synapse-core::renderer";
    std::string section = "[Window][" + imgui_id + "]";
    std::cout << "searching for '" << section << "'.\n";
    std::string pos, size;
    std::string line;
    
    class ivec2
    {
    public:
        int x;
        int y;
        ivec2() {}
        ivec2(int _x, int _y) :
            x(_x), y(_y) {}
        void print() { std::cout << "[" << x << ", " << y << "]\n"; }
    };

    
    ivec2 vpos, vsize;

    while(std::getline(fs, line))
    {
        // looking for correct section
        if (section.compare(0, section.size(), line) == 0)
        {
            // find Pos and Size and get values into vectors
            //
            std::getline(fs, pos);
            pos = pos.substr(pos.find('=')+1);
            vpos.x = std::stoi(pos.substr(0, pos.find(',')));
            vpos.y = std::stoi(pos.substr(pos.find(',')+1));
            //
            std::getline(fs, size);
            size = size.substr(size.find('=')+1);
            vsize.x = std::stoi(size.substr(0, size.find(',')));
            vsize.y = std::stoi(size.substr(size.find(',')+1));

            break;
        }
    }

    vpos.print();
    vsize.print();

    ivec2 res(vsize.x-vpos.x, vsize.y-vpos.y);
    res.print();
    
    return 0;

}
