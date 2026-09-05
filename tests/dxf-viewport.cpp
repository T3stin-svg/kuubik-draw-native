// Camera-record regression only; full layout ownership is tested separately.
#include "dxf-test-interface.h"
#include <cmath>
#include <vector>

class Viewports : public DxfTestInterface {
public:
    dxfRW* writer = nullptr;
    std::vector<DRW_Viewport> viewports;
    std::vector<DRW_Line> lines;
    std::vector<DRW_Layer> layers;
    void addViewport(const DRW_Viewport& data) override { viewports.push_back(data); }
    void addLine(const DRW_Line& data) override { lines.push_back(data); }
    void addLayer(const DRW_Layer& data) override { layers.push_back(data); }
    void writeHeader(DRW_Header& data) override { data.addInt("$INSUNITS", 4, 70); }
    void writeLayers() override { for (auto& layer : layers) writer->writeLayer(&layer); }
    void writeEntities() override {
        for (auto& line : lines) assert(writer->writeLine(&line));
        for (auto& viewport : viewports) assert(writer->writeViewport(&viewport));
    }
};

void checkCameras(const Viewports& data) {
    assert(data.viewports.size() == 3 && data.lines.size() == 1);
    int seen = 0;
    for (const auto& viewport : data.viewports) {
        assert(viewport.vpID >= 1 && viewport.vpID <= 3);
        assert(!(seen & (1 << viewport.vpID)));
        seen |= 1 << viewport.vpID;
        if (viewport.vpID == 1) continue;
        const int index = viewport.vpID - 2;
        assert(viewport.pswidth == 160 && viewport.psheight == 160);
        assert(viewport.basePoint.x == 100 + index * 190 && viewport.basePoint.y == 148.5);
        assert(viewport.centerPX == 2500 - index * 1250 && viewport.centerPY == index * 800);
        assert(viewport.viewHeight == 8000 + index * 8000);
        assert(viewport.vpFlags == (0x8000 | 0x200 | (index == 0 ? 0x4000 : 0)));
        assert(viewport.viewTarget.x == index * 100 && viewport.viewTarget.y == index * 200);
        assert(viewport.viewTarget.z == 0);
        assert(viewport.viewDir.x == index && viewport.viewDir.y == index * 2);
        assert(viewport.viewDir.z == 1 + index * 2);
        assert(std::abs(viewport.twistAngle - index * M_PI / 6) < 1e-10);
    }
}

int main(int argc, char** argv) {
    assert(argc == 3);
    const DRW_Viewport defaults;
    assert(defaults.vpID == 0 && defaults.vpFlags == 0 && defaults.viewHeight > 0);
    assert(defaults.viewDir.z == 1 && defaults.twistAngle == 0);
    Viewports input;
    dxfRW source(argv[1]);
    assert(source.read(&input, false));
    checkCameras(input);
    dxfRW output(argv[2]);
    input.writer = &output;
    assert(output.write(&input, DRW::AC1032, false));
    Viewports reopened;
    assert(output.read(&reopened, false));
    checkCameras(reopened);
}
