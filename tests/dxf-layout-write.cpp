// Bounded layout export regression; source-file support is gated by the Python driver.
#include "dxf-test-interface.h"
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

std::string bytes(const char* path) {
    std::ifstream input(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
}

class LayoutDocument : public DxfTestInterface {
public:
    dxfRW* writer = nullptr;
    int units = 0, headerCalls = 0, layerCalls = 0;
    bool legacy = false;
    bool failHeader = false, failLayers = false, failLegacyImage = false;
    std::vector<DRW_Layout> layouts;
    std::vector<DRW_Block_Record> blocks;
    std::vector<DRW_Viewport> viewports;
    std::vector<DRW_Line> lines;
    std::vector<DRW_Layer> layers;
    void addHeader(const DRW_Header* data) override {
        const auto found = data->vars.find("$INSUNITS");
        if (found != data->vars.end() && found->second->type() == DRW_Variant::INTEGER)
            units = found->second->content.i;
    }
    void addLayout(const DRW_Layout& data) override { layouts.emplace_back(data); }
    void addBlockRecord(const DRW_Block_Record& data) override { blocks.emplace_back(data); }
    void addViewport(const DRW_Viewport& data) override { viewports.emplace_back(data); }
    void addLine(const DRW_Line& data) override { lines.emplace_back(data); }
    void addLayer(const DRW_Layer& data) override { layers.emplace_back(data); }
    void writeHeader(DRW_Header& data) override {
        ++headerCalls;
        if (failHeader) throw std::runtime_error("injected header failure");
        if (units) data.addInt("$INSUNITS", units, 70);
    }
    void writeLayers() override {
        ++layerCalls;
        for (auto& layer : layers) assert(writer->writeLayer(&layer));
        if (failLayers) throw std::runtime_error("injected serialization failure");
    }
    void writeEntities() override {
        assert(legacy); // Explicit export must use only its validated entity list.
        if (failLegacyImage) {
            DRW_Image image;
            image.sizeu = image.sizev = 1;
            assert(writer->writeImage(&image, "synthetic-unused.png"));
            throw std::runtime_error("injected legacy image failure");
        }
        for (auto& line : lines) assert(writer->writeLine(&line));
    }
    std::vector<const DRW_Entity*> entities() const {
        std::vector<const DRW_Entity*> result;
        for (const auto& line : lines) result.push_back(&line);
        for (const auto& viewport : viewports) result.push_back(&viewport);
        return result;
    }
};

int main(int argc, char** argv) {
    assert(argc == 3 || argc == 4);
    LayoutDocument input;
    dxfRW source(argv[1]);
    assert(source.read(&input, false));
    assert(input.layouts.size() == 2 && input.blocks.size() == 2);
    assert(input.viewports.size() == 3 && input.lines.size() == 1);
    const std::string mode = argc == 4 ? argv[3] : "write";
    auto& viewport = input.viewports.back();
    if (mode == "duplicate-handle") viewport.handle = input.lines.front().handle;
    else if (mode == "duplicate-viewport-id") viewport.vpID = input.viewports.front().vpID;
    else if (mode == "wrong-owner") viewport.parentHandle = input.lines.front().parentHandle;
    else if (mode == "wrong-backlink") input.blocks.front().layoutHandle = input.layouts.back().handle;
    else if (mode == "wrong-last-viewport") input.layouts.back().lastViewportHandle = input.lines.front().handle;
    else if (mode == "missing-units") input.units = 0;
    else if (mode == "inches") input.units = 1;
    else if (mode == "nan-camera") viewport.viewHeight = std::numeric_limits<double>::quiet_NaN();
    else if (mode == "negative-camera") viewport.psheight = -1;
    else if (mode == "perspective") viewport.vpFlags |= 1;
    else if (mode == "nonrectangular") viewport.vpFlags |= 0x10000;
    else if (mode == "nonplanar") viewport.viewDir.x = 1;
    else if (mode == "handle-overflow") input.lines.front().handle = std::numeric_limits<int>::max() - 10;
    else if (mode == "handle-exhaustion") input.lines.front().handle = std::numeric_limits<int>::max() - 51;
    else if (mode == "angle-overflow") viewport.twistAngle = std::numeric_limits<double>::max();
    else if (mode == "newline-name") input.layouts.back().name = "bad\n0\nEOF";
    else if (mode == "wrong-type") input.lines.front().eType = DRW::VIEWPORT;
    else if (mode == "unsupported-entity") input.lines.front().eType = DRW::CIRCLE;
    else if (mode == "header-exception") input.failHeader = true;
    else if (mode == "write-exception") input.failLayers = true;
    dxfRW output(argv[2]);
    input.writer = &output;
    if (mode == "legacy-image-retry") {
        input.legacy = input.failLegacyImage = true;
        assert(!output.write(&input, DRW::AC1032, false));
        input.legacy = input.failLegacyImage = false;
        input.headerCalls = 0;
    }
#ifdef _WIN32
    HANDLE locked = INVALID_HANDLE_VALUE;
    if (mode == "locked-target") {
        locked = CreateFileA(argv[2], GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
        assert(locked != INVALID_HANDLE_VALUE);
    }
#endif
    const auto previous = bytes(argv[2]);
    const bool written = output.writeLayoutDocument(&input, input.layouts, input.blocks, input.entities());
#ifdef _WIN32
    if (locked != INVALID_HANDLE_VALUE) assert(CloseHandle(locked));
#endif
    assert(input.headerCalls == 1);
    if (mode == "handle-exhaustion") assert(input.layerCalls == 1);
    if (mode == "handle-overflow") assert(input.layerCalls == 0);
    if (mode != "write" && mode != "replace-failure" && mode != "legacy-image-retry") {
        assert(!written && output.getError() != DRW::BAD_NONE);
        assert(bytes(argv[2]) == previous);
        // Reuse after a rejected explicit operation, then return to the legacy path.
        LayoutDocument valid;
        assert(source.read(&valid, false));
        valid.writer = &output;
        assert(output.writeLayoutDocument(&valid, valid.layouts, valid.blocks, valid.entities()));
        LayoutDocument reopened;
        assert(output.read(&reopened, false));
        assert(reopened.layouts.size() == 2 && reopened.viewports.size() == 3);
        valid.legacy = true;
        assert(output.write(&valid, DRW::AC1032, false));
        LayoutDocument oldPath;
        assert(output.read(&oldPath, false));
        assert(oldPath.layouts.empty() && oldPath.viewports.empty() && oldPath.lines.size() == 1);
        std::cout << "PASS rejected input, valid retry and legacy reuse: " << mode << '\n';
        return 0;
    }
    if (!written) {
        assert(bytes(argv[2]) == previous);
        return 3;
    }
    LayoutDocument reopened;
    assert(output.read(&reopened, false));
    assert(reopened.layouts.size() == 2 && reopened.viewports.size() == 3 && reopened.lines.size() == 1);
    std::cout << "PASS native layout export and reopen\n";
}
