// Common attributes must reach consumers even when geometry is unsupported.
#include "dxf-test-interface.h"
#include <iostream>

class ContextReader : public DxfTestInterface {
public:
    std::vector<DRW_Line> lines;
    dxfRW* writer = nullptr;
    duint32 observedHandle = 0, observedOwner = 0;
    void addPolyline(const DRW_Polyline& line) override {
        assert(line.handle == observedHandle && line.parentHandle == observedOwner);
    }
    void addLine(const DRW_Line& line) override { lines.push_back(line); }
    void writeEntities() override {
        for (auto& line : lines) assert(writer->writeLine(&line));
    }
    void addEntity(const DRW_Entity& entity) override {
        observedHandle = entity.handle;
        observedOwner = entity.parentHandle;
        std::cout << entity.handle << ' ' << entity.parentHandle << ' '
                  << static_cast<int>(entity.space) << '\n';
    }
};

int main(int argc, char** argv) {
    assert(argc == 2 || argc == 3);
    ContextReader context;
    dxfRW input(argv[1]);
    if (!input.read(&context, false)) return 2;
    if (argc == 3) {
        dxfRW output(argv[2]);
        context.writer = &output;
        const bool binary = std::string(argv[2]).find(".bdxf") != std::string::npos;
        assert(output.write(&context, DRW::AC1032, binary));
    }
    return 0;
}
