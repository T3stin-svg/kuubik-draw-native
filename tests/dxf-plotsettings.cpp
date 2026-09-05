// Synthetic modelspace adapter for the libdxfrw ownership regression (GPLv2).
// Build against the repository's libdxfrw; no Qt or user settings are involved.
#include "libdxfrw.h"
#include <cassert>
#include <string>
#include <vector>

#ifdef NDEBUG
#error Build this regression adapter with assertions enabled (-UNDEBUG or /UNDEBUG).
#endif

class Modelspace : public DRW_Interface {
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

    void addHeader(const DRW_Header*) override {}
    void addLType(const DRW_LType&) override {}
    void addDimStyle(const DRW_Dimstyle&) override {}
    void addVport(const DRW_Vport&) override {}
    void addTextStyle(const DRW_Textstyle&) override {}
    void addAppId(const DRW_AppId&) override {}
    void addBlock(const DRW_Block&) override {}
    void setBlock(int) override {}
    void endBlock() override {}
    void addPoint(const DRW_Point&) override {}
    void addRay(const DRW_Ray&) override {}
    void addXline(const DRW_Xline&) override {}
    void addArc(const DRW_Arc&) override {}
    void addEllipse(const DRW_Ellipse&) override {}
    void addPolyline(const DRW_Polyline&) override {}
    void addSpline(const DRW_Spline*) override {}
    void addKnot(const DRW_Entity&) override {}
    void addInsert(const DRW_Insert&) override {}
    void addTrace(const DRW_Trace&) override {}
    void add3dFace(const DRW_3Dface&) override {}
    void addSolid(const DRW_Solid&) override {}
    void addMText(const DRW_MText&) override {}
    void addText(const DRW_Text&) override {}
    void addDimAlign(const DRW_DimAligned*) override {}
    void addDimLinear(const DRW_DimLinear*) override {}
    void addDimRadial(const DRW_DimRadial*) override {}
    void addDimDiametric(const DRW_DimDiametric*) override {}
    void addDimAngular(const DRW_DimAngular*) override {}
    void addDimAngular3P(const DRW_DimAngular3p*) override {}
    void addDimOrdinate(const DRW_DimOrdinate*) override {}
    void addLeader(const DRW_Leader*) override {}
    void addHatch(const DRW_Hatch*) override {}
    void addViewport(const DRW_Viewport&) override {}
    void addImage(const DRW_Image*) override {}
    void linkImage(const DRW_ImageDef*) override {}
    void addComment(const char*) override {}
    void writeBlocks() override {}
    void writeBlockRecords() override {}
    void writeLTypes() override {}
    void writeTextstyles() override {}
    void writeVports() override {}
    void writeDimstyles() override {}
    void writeAppId() override {}
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
