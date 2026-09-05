// Synthetic modelspace adapter for the libdxfrw ownership regression (GPLv2).
// Build against the repository's libdxfrw; no Qt or user settings are involved.
#include "dxf-test-interface.h"
#include <string>
#include <vector>

class Modelspace : public DxfTestInterface {
public:
    dxfRW* writer = nullptr;
    int plotCount = 1;
    int plotsRead = 0;
    std::vector<DRW_Line> lines;
    std::vector<DRW_Circle> circles;
    std::vector<DRW_LWPolyline> polylines;
    std::vector<DRW_Layer> layers;

    void addLine(const DRW_Line& data) override { lines.push_back(data); }
    void addCircle(const DRW_Circle& data) override { circles.push_back(data); }
    void addLWPolyline(const DRW_LWPolyline& data) override { polylines.push_back(data); }
    void addLayer(const DRW_Layer& data) override { layers.push_back(data); }
    void addPlotSettings(const DRW_PlotSettings* data) override {
        assert(data->plotViewName == std::to_string(plotsRead + 1) + "x1");
        assert(data->marginLeft == 1 && data->marginBottom == 2
               && data->marginRight == 3 && data->marginTop == 4);
        ++plotsRead;
    }
    void writeHeader(DRW_Header& data) override { data.addInt("$INSUNITS", 4, 70); }
    void writeEntities() override {
        for (auto& entity : lines) writer->writeLine(&entity);
        for (auto& entity : circles) writer->writeCircle(&entity);
        for (auto& entity : polylines) writer->writeLWPolyline(&entity);
    }
    void writeLayers() override { for (auto& layer : layers) writer->writeLayer(&layer); }
    void writeObjects() override {
        for (int i = 0; i < plotCount; ++i) {
            DRW_PlotSettings settings;
            settings.plotViewName = std::to_string(i + 1) + "x1";
            settings.marginLeft = 1;
            settings.marginBottom = 2;
            settings.marginRight = 3;
            settings.marginTop = 4;
            assert(writer->writePlotSettings(&settings));
        }
    }

};

int main(int argc, char** argv) {
    assert(argc == 3 || argc == 4);
    Modelspace model;
    dxfRW input(argv[1]);
    assert(input.read(&model, false));
    assert(model.lines.size() == 1 && model.circles.size() == 1 && model.polylines.size() == 1);
    // Optional binary probe is separate: the inherited binary header is malformed.
    const std::vector<std::string> modes = argc == 4
        ? std::vector<std::string>{"binary"}
        : std::vector<std::string>{"one", "empty", "two", "reused"};
    for (const auto& mode : modes) {
        const std::string path = std::string(argv[2]) + "/" + mode + ".dxf";
        dxfRW output(path.c_str());
        model.writer = &output;
        model.plotCount = mode == "empty" ? 0 : mode == "two" ? 2 : 1;
        if (mode == "reused") {
            model.plotCount = 2;
            assert(output.write(&model, DRW::AC1027, false));
            model.plotCount = 1;
        }
        assert(output.write(&model, DRW::AC1027, mode == "binary"));
        Modelspace reopened;
        assert(output.read(&reopened, false));
        assert(reopened.lines.size() == 1 && reopened.circles.size() == 1
               && reopened.polylines.size() == 1 && reopened.plotsRead == model.plotCount);
    }
}
