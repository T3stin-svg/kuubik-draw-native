// Bounded DXF layout reader contract; no application settings or drawing changes.
#include "dxf-test-interface.h"
#include <iomanip>
#include <iostream>

class LayoutReader : public DxfTestInterface {
public:
    int layouts = 0, records = 0;
    void addBlockRecord(const DRW_Block_Record& data) override {
        ++records;
        std::cout << "BLOCK_RECORD\t" << data.name << '\t' << data.handle << '\t'
                  << data.parentHandle << '\t' << data.layoutHandle << '\t' << data.insUnits << '\n';
    }
    void addLayout(const DRW_Layout& data) override {
        ++layouts;
        std::cout << "LAYOUT\t" << data.name << '\t' << data.handle << '\t'
                  << data.parentHandle << '\t' << data.blockRecordHandle << '\t'
                  << data.lastViewportHandle << '\t' << data.tabOrder << '\t'
                  << data.pageSetupName << '\t' << data.paperWidth << '\t'
                  << data.paperHeight << '\t' << data.paperUnits << '\t'
                  << data.paperRotation << '\t' << data.scaleNumerator << '\t'
                  << data.scaleDenominator << '\t' << data.plotFlags << '\t'
                  << data.layoutFlags << '\t' << data.reactors.size();
        for (auto handle : data.reactors) std::cout << '\t' << handle;
        std::cout << '\t' << data.printerName << '\t' << data.paperSize << '\t' << data.plotViewName
                  << '\t' << data.marginLeft << '\t' << data.marginBottom
                  << '\t' << data.marginRight << '\t' << data.marginTop
                  << '\t' << data.plotOriginX << '\t' << data.plotOriginY
                  << '\t' << data.plotType << '\t' << data.standardScaleType
                  << '\t' << data.standardScale << '\t' << data.paperOriginX << '\t' << data.paperOriginY;
        for (const auto& point : {data.minLimit, data.maxLimit, data.basePoint, data.minExtent,
                                  data.maxExtent, data.ucsOrigin, data.ucsXAxis, data.ucsYAxis})
            std::cout << '\t' << point.x << '\t' << point.y << '\t' << point.z;
        std::cout << '\t' << data.elevation << '\t' << data.ucsType
                  << '\t' << data.ucsHandle << '\t' << data.baseUcsHandle;
        std::cout << '\n';
    }
};

int main(int argc, char** argv) {
    assert(argc == 2);
    // Retained records use deep copy construction, including owned XDATA.
    DRW_Block_Record original;
    original.extData.push_back(new DRW_Variant(1000, std::string("retained")));
    std::vector<DRW_Block_Record> retained;
    retained.push_back(original);
    original.reset();
    assert(retained[0].extData.size() == 1);
    assert(*retained[0].extData[0]->content.s == "retained");
    std::cout << std::setprecision(17);
    LayoutReader model;
    dxfRW input(argv[1]);
    if (!input.read(&model, false)) return 2;
    assert(model.layouts == 2 && model.records >= 2);
}
