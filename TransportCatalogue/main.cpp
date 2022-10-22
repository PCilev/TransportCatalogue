#include <fstream>
#include <iostream>
#include <string_view>

#include "json_reader.h"
#include "map_renderer.h"
#include "transport_router.h"

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);
    
    TrC::TransportCatalogue catalogue;
    renderer::MapRenderer render;
    router::Graph graph;
    RequestHandler handler(catalogue, render, graph);

    if (mode == "make_base"sv) {
        JsonReader read(std::cin);
        read.FillingCatalogue(catalogue, handler);
        render.SetSettings(read.ReadRenderSettings());
        handler.GraphInit(read.ReadRoutingSettings());
        
        handler.Serialize(read.ReadSerializationSettings(), read.ReadRoutingSettings());

    } else if (mode == "process_requests"sv) {
        JsonReader read(std::cin);
        router::TransportRouter router(catalogue, graph);
        if (handler.Deserialize(read.ReadSerializationSettings(), router)) {
            json::Print(read.MakeReport(handler, router), std::cout);
        }
        else {
            std::cerr << "Deserialize ERROR" << std::endl;
        }

    } else {
        PrintUsage();
        return 1;
    }
}