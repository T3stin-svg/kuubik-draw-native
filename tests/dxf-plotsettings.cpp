// Synthetic modelspace adapter for the libdxfrw ownership regression (GPLv2).
// Build against the repository's libdxfrw; no Qt or user settings are involved.
#include "dxf-test-interface.h"
#include <string>
#include <vector>

const std::vector<std::string> binaryFlags = {
    "$LWDISPLAY", "$XEDIT", "$PSTYLEMODE", "$EXTNAMES", "$OLESTARTUP",
    "$XCLIPFRAME", "$CAMERADISPLAY", "$REALWORLDSCALE"
};
const std::unordered_map<std::string, int> defaultFlags = {
    {"$LWDISPLAY", 0}, {"$XEDIT", 1}, {"$PSTYLEMODE", 1}, {"$EXTNAMES", 1},
    {"$OLESTARTUP", 0}, {"$XCLIPFRAME", 0}, {"$CAMERADISPLAY", 0}, {"$REALWORLDSCALE", 1}
};

class Modelspace : public DxfTestInterface {
public:
    dxfRW* writer = nullptr;
    int plotCount = 1;
    int plotsRead = 0;
    int booleanValue = -1, dimfxRead = -1;
    std::unordered_map<std::string, int> flagsRead;
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
    void addHeader(const DRW_Header* data) override {
        for (const auto& key : binaryFlags) {
            const auto it = data->vars.find(key);
            if (it != data->vars.end()) {
                assert(it->second->type() == DRW_Variant::INTEGER);
                flagsRead[key] = it->second->content.i;
            }
        }
    }
    void addDimStyle(const DRW_Dimstyle& data) override {
        if (data.name == "KUUBIK_BINARY") dimfxRead = data.dimfxlon;
    }
    void writeHeader(DRW_Header& data) override {
        data.addInt("$INSUNITS", 4, 70);
        if (booleanValue >= 0) for (const auto& key : binaryFlags) data.addInt(key, booleanValue, 290);
    }
    void writeDimstyles() override {
        if (booleanValue < 0) return;
        DRW_Dimstyle style;
        style.name = "KUUBIK_BINARY";
        style.dimfxlon = booleanValue;
        assert(writer->writeDimstyle(&style));
    }
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
    if (argc == 4) {
        for (const auto& version : std::vector<std::pair<std::string, DRW::Version>>{
            {"AC1015", DRW::AC1015}, {"AC1018", DRW::AC1018}, {"AC1021", DRW::AC1021}, {"AC1027", DRW::AC1027}, {"AC1032", DRW::AC1032}}) {
            for (int value : {-1, 0, 1}) {
                const std::string path = std::string(argv[2]) + "/" + version.first + "-" + (value < 0 ? "default" : std::to_string(value)) + ".dxf";
                dxfRW output(path.c_str());
                model.writer = &output;
                model.booleanValue = value;
                assert(output.write(&model, version.second, true));
                Modelspace reopened;
                assert(output.read(&reopened, false));
                assert(reopened.lines.size() == 1 && reopened.circles.size() == 1 && reopened.polylines.size() == 1 && reopened.plotsRead == 1);
                assert(reopened.flagsRead.size() == (version.second == DRW::AC1015 ? 5 : version.second == DRW::AC1018 ? 6 : 8));
                for (const auto& flag : reopened.flagsRead) assert(flag.second == (value < 0 ? defaultFlags.at(flag.first) : value));
                assert(reopened.dimfxRead == (value < 0 ? -1 : version.second <= DRW::AC1018 ? 0 : value));
            }
        }
        return 0;
    }
    const std::vector<std::string> modes = {"one", "empty", "two", "reused"};
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
