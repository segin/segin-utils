#include <stdio.h>
#include <stdlib.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <unistd.h>
#include <string.h>

void parse_series_xml(const char* xml_file_path, const char* csv_file_path) {
    xmlDocPtr doc;
    xmlNodePtr root, series;

    // Parse the XML file
    doc = xmlReadFile(xml_file_path, NULL, 0);

    if (doc == NULL) {
        fprintf(stderr, "Error: Could not parse XML file.\n");
        return;
    }

    // Open CSV file for writing
    FILE* csv_file = fopen(csv_file_path, "w");
    if (csv_file == NULL) {
        fprintf(stderr, "Error: Could not open CSV file for writing.\n");
        xmlFreeDoc(doc);
        return;
    }

    // Write header row to CSV file
    fprintf(csv_file, "id,imdbId,TMDBCode,SeriesName\n");

    // Get the root element
    root = xmlDocGetRootElement(doc);

    // Loop through Series tags
    for (series = root->children; series; series = series->next) {
        if (series->type == XML_ELEMENT_NODE && xmlStrEqual(series->name, BAD_CAST "Series")) {
            // Extract data
            xmlNodePtr id_node = xmlFirstElementChild(series);
            xmlNodePtr imdbId_node = id_node->next;
            xmlNodePtr TMDBCode_node = imdbId_node->next;
            xmlNodePtr SeriesName_node = TMDBCode_node->next;

            // Write data to CSV file
            fprintf(csv_file, "%s,%s,%s,%s\n", id_node->children->content,
                    imdbId_node->children->content, TMDBCode_node->children->content,
                    SeriesName_node->children->content);
        }
    }

    // Close CSV file
    fclose(csv_file);

    // Free the XML document
    xmlFreeDoc(doc);
}

void parse_myshows_xml(const char* xml_file_path, const char* csv_file_path) {
    xmlDocPtr doc;
    xmlNodePtr root, show_item;

    // Parse the XML file
    doc = xmlReadFile(xml_file_path, NULL, 0);

    if (doc == NULL) {
        fprintf(stderr, "Error: Could not parse XML file.\n");
        return;
    }

    // Open CSV file for writing
    FILE* csv_file = fopen(csv_file_path, "w");
    if (csv_file == NULL) {
        fprintf(stderr, "Error: Could not open CSV file for writing.\n");
        xmlFreeDoc(doc);
        return;
    }

    // Write header row to CSV file
    fprintf(csv_file, "TVDBID,ImdbCode,TMDBID,LastName\n");

    // Get the root element
    root = xmlDocGetRootElement(doc);

    // Loop through ShowItem tags
    for (show_item = root->children; show_item; show_item = show_item->next) {
        if (show_item->type == XML_ELEMENT_NODE && xmlStrEqual(show_item->name, BAD_CAST "ShowItem")) {
            // Extract data
            xmlNodePtr tvdb_id_node = xmlFirstElementChild(show_item);
            xmlNodePtr imdb_code_node = tvdb_id_node->next;
            xmlNodePtr tmdb_id_node = imdb_code_node->next;
            xmlNodePtr last_name_node = tmdb_id_node->next;

            // Write data to CSV file
            fprintf(csv_file, "%s,%s,%s,%s\n", tvdb_id_node->children->content,
                    imdb_code_node->children->content, tmdb_id_node->children->content,
                    last_name_node->children->content);
        }
    }

    // Close CSV file
    fclose(csv_file);

    // Free the XML document
    xmlFreeDoc(doc);
}

int main(int argc, char *argv[]) {
    int option;
    const char* xml_file_path = NULL;
    const char* csv_file_path = NULL;

    // Check if the correct number of command-line arguments is provided
    if (argc != 7) {
        fprintf(stderr, "Usage: %s -t <series|myshows> -x <path_to_xml_file> -c <path_to_csv_file>\n", argv[0]);
        return 1;
    }

    // Parse command-line options
    while ((option = getopt(argc, argv, "t:x:c:")) != -1) {
        switch (option) {
            case 't':
                if (strcmp(optarg, "series") == 0) {
                    parse_series_xml(xml_file_path, csv_file_path);
                } else if (strcmp(optarg, "myshows") == 0) {
                    parse_myshows_xml(xml_file_path, csv_file_path);
                } else {
                    fprintf(stderr, "Error: Invalid value for option -t\n");
                    return 1;
                }
                break;
            case 'x':
                xml_file_path = optarg;
                break;
            case 'c':
                csv_file_path = optarg;
                break;
            default:
                fprintf(stderr, "Usage: %s -t <series|myshows> -x <path_to_xml_file> -c <path_to_csv_file>\n", argv[0]);
                return 1;
        }
    }

    return 0;
}
