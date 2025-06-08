#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cstring>

// Used for flippling the image
// The reverse function is used to reverse the imageData vector in the TGA struct
#include <algorithm>

using namespace std;

struct Pixel
{
    // blue, green, red;
    unsigned char data[3]; 
};

struct TGA
{
    // Header
    char idLength;
    char colorMapType;
    char dataTypeCode;
    short colorMapOrigin;
    short colorMapLength;
    char colorMapDepth;
    short xOrigin;
    short yOrigin;
    short width;
    short height;
    char bitsPerPixel;
    char imageDescriptor;

    // Pixels
    vector <Pixel> imageData;
};

TGA readTGA(string const &filename, 
            string const & errorMessage = "Error: Cannot open TGA File")
{
    // Open the tga file in binary mode
    ifstream file(filename, std::ios::binary);

    // Make sure the file is opened successfully
    if ( !file.is_open() ) {
        cout << errorMessage << endl;
        exit(1);
    }

    TGA tga;

    // Read the header data

    file.read((char*)&tga.idLength, 1);
    file.read((char*)&tga.colorMapType, 1);
    file.read((char*)&tga.dataTypeCode, 1);
    file.read((char*)&tga.colorMapOrigin, 2);
    file.read((char*)&tga.colorMapLength, 2);
    file.read((char*)&tga.colorMapDepth, 1);
    file.read((char*)&tga.xOrigin, 2);
    file.read((char*)&tga.yOrigin, 2);
    file.read((char*)&tga.width, 2);
    file.read((char*)&tga.height, 2);
    file.read((char*)&tga.bitsPerPixel, 1);
    file.read((char*)&tga.imageDescriptor, 1);

    // Read all the image data pixels
    int numPixels = tga.width * tga.height;

    for (int i = 0; i < numPixels; ++i) {

        // Pixels are arranged in BGR format
        Pixel pixel;
        file.read((char*) &pixel.data[0], 1);
        file.read((char*) &pixel.data[1], 1);
        file.read((char*) &pixel.data[2], 1);

        tga.imageData.push_back(pixel);
    }

    return tga;
}

void writeTGA(const TGA & tga, const string & filename)
{
    // Open the new file in binary mode
    ofstream file(filename, std::ios::binary);

    // Make sure the file is opened successfully
    if ( !file.is_open() ) {
        cerr << "Error: Writing TGA File" << endl;
        cerr << "Error: Cannot open TGA File: " << filename << endl;
        cerr << "Exiting the program" << endl;
        exit(1);
    }

    // Write the header

    file.write((char*)&tga.idLength, 1);
    file.write((char*)&tga.colorMapType, 1);
    file.write((char*)&tga.dataTypeCode, 1);
    file.write((char*)&tga.colorMapOrigin, 2);
    file.write((char*)&tga.colorMapLength, 2);
    file.write((char*)&tga.colorMapDepth, 1);
    file.write((char*)&tga.xOrigin, 2);
    file.write((char*)&tga.yOrigin, 2);
    file.write((char*)&tga.width, 2);
    file.write((char*)&tga.height, 2);
    file.write((char*)&tga.bitsPerPixel, 1);
    file.write((char*)&tga.imageDescriptor, 1);

    // Write all the image data pixels
    int numPixels = tga.width * tga.height;

    for (int i = 0; i < numPixels; ++i) {

        // Pixels are arranged in BGR format
        Pixel pixel = tga.imageData[i];

        file.write((char*) &pixel.data[0], 1);
        file.write((char*) &pixel.data[1], 1);
        file.write((char*) &pixel.data[2], 1);
    }
}

int clamp(int v) 
{
    if (v < 0) {
        v = 0;
    }

    if (v > 255) {
        v = 255;
    }

    return v;
}

TGA operationMultiply(TGA const &lhs, TGA const &rhs)
{
    // lhs and rhs have same header and so does the resultant tga file
    TGA output = lhs;

    int numPixels = lhs.width * lhs.height;

    for (int i = 0; i < numPixels; ++i) {

        for (int j = 0; j < 3; ++j) {
            float a = (float) lhs.imageData[i].data[j];
            float b = (float) rhs.imageData[i].data[j];

            a /= 255.0;
            b /= 255.0;

            output.imageData[i].data[j] = (unsigned char)(int)(((a * b) * 255.0) + 0.5);
        }
    }

    return output;
}

TGA operationAddition(TGA const & lhs, TGA const & rhs)
{
    // lhs and rhs have same header and so does the resultant tga file
    TGA output = lhs;

    int numPixels = lhs.width * lhs.height;

    for (int i = 0; i < numPixels; ++i) {

        for (int j = 0; j < 3; ++j) {
            int a = lhs.imageData[i].data[j];
            int b = rhs.imageData[i].data[j];

            int r = (a + b);

            r = clamp(r);

            output.imageData[i].data[j] = (unsigned char)r;
        }
    }

    return output;
}

TGA operationSubtraction(TGA const & lhs, TGA const & rhs)
{
    // lhs and rhs have same header and so does the resultant tga file
    TGA output = lhs;

    int numPixels = lhs.width * lhs.height;

    for (int i = 0; i < numPixels; ++i) {

        for (int j = 0; j < 3; ++j) {
            int a = lhs.imageData[i].data[j];
            int b = rhs.imageData[i].data[j];

            int r = (a - b);

            r = clamp(r);

            output.imageData[i].data[j] = (unsigned char)r;
        }
    }

    return output;
}


TGA operationScreen(TGA const &lhs, TGA const &rhs)
{
    // lhs and rhs have same header and so does the resultant tga file
    TGA output = lhs;

    int numPixels = lhs.width * lhs.height;

    for (int i = 0; i < numPixels; ++i) {

        for (int j = 0; j < 3; ++j) {
            float a = (float) lhs.imageData[i].data[j];
            float b = (float) rhs.imageData[i].data[j];

            a /= 255.0;
            b /= 255.0;

            float r = 1 - ((1 - a) * (1 - b));

            output.imageData[i].data[j] = (unsigned char)(int)((r * 255.0) + 0.5);
        }
    }

    return output;
}


TGA operationOverlay(TGA const &lhs, TGA const &rhs)
{
    // lhs and rhs have same header and so does the resultant tga file
    TGA output = lhs;

    int numPixels = lhs.width * lhs.height;

    for (int i = 0; i < numPixels; ++i) {

        for (int j = 0; j < 3; ++j) {
            float a = (float) lhs.imageData[i].data[j];
            float b = (float) rhs.imageData[i].data[j];

            a /= 255.0;
            b /= 255.0;

            float r = 0.0;

            if (b <= 0.5) {
                r = 2 * a * b;
            }
            else {
                r = 1 - (2 * (1 - a) * (1 - b));
            }

            output.imageData[i].data[j] = (unsigned char)(int)((r * 255.0) + 0.5);
        }
    }

    return output;
}

TGA operationAddition(TGA const & tga, int red, int green, int blue)
{
    // tga and the output will have the same header and imageData dimension
    TGA output = tga;

    int numPixels = tga.width * tga.height;

    for (int i = 0; i < numPixels; ++i) {
        
        int value[3] = {
            blue, green, red
        };

        for (int j = 0; j < 3; ++j) {
            int a = tga.imageData[i].data[j];
            int b = value[j];

            int r = (a + b);

            r = clamp(r);

            output.imageData[i].data[j] = (unsigned char)r;
        }
    }

    return output;
}



TGA operationScale(TGA const & tga, int red, int green, int blue)
{
    // tga and the output will have the same header and imageData dimension
    TGA output = tga;

    int numPixels = tga.width * tga.height;

    for (int i = 0; i < numPixels; ++i) {
        
        int value[3] = {
            blue, green, red
        };

        for (int j = 0; j < 3; ++j) {
            int a = tga.imageData[i].data[j];
            int b = value[j];

            int r = (a * b);

            r = clamp(r);

            output.imageData[i].data[j] = (unsigned char)r;
        }
    }

    return output;
}



TGA operationOnly(TGA const & tga, bool red, bool green, bool blue)
{
    // tga and the output will have the same header and imageData dimension
    TGA output = tga;

    int numPixels = tga.width * tga.height;

    for (int i = 0; i < numPixels; ++i) {
        
        // order = blue, green, red
        if (red) {
            output.imageData[i].data[0] = tga.imageData[i].data[2];
            output.imageData[i].data[1] = tga.imageData[i].data[2];
            output.imageData[i].data[2] = tga.imageData[i].data[2];
        }
        else if (green) {
            output.imageData[i].data[0] = tga.imageData[i].data[1];
            output.imageData[i].data[1] = tga.imageData[i].data[1];
            output.imageData[i].data[2] = tga.imageData[i].data[1];
        }
        else if (blue) {
            output.imageData[i].data[0] = tga.imageData[i].data[0];
            output.imageData[i].data[1] = tga.imageData[i].data[0];
            output.imageData[i].data[2] = tga.imageData[i].data[0];
        }
    }

    return output;
}


TGA operationCombine(TGA const & red, TGA const & green, TGA const & blue)
{
    // tga and the output will have the same header and imageData dimension
    TGA output = red;

    int numPixels = red.width * red.height;

    for (int i = 0; i < numPixels; ++i) {
        output.imageData[i].data[0] = blue.imageData[i].data[0];
        output.imageData[i].data[1] = green.imageData[i].data[1];
        output.imageData[i].data[2] = red.imageData[i].data[2];
    }

    return output;
}


TGA operationFlip(TGA const & tga)
{
    // tga and the output will have the same header and imageData dimension
    TGA output = tga;

    reverse(output.imageData.begin(), output.imageData.end());

    return output;
}

void printUsage()
{
    cout << "Project 2: Image Processing, Spring 2023" << endl;
    cout << endl;
    cout << "Usage:" << endl;
    cout << "\t./project2.out [output] [firstImage] [method] [...]" << endl;
}

bool stringEndingWith(string const & src, string const & extension)
{
    if (src.size() >= extension.size()) {
        const char * start = src.c_str() + (src.size() - extension.size());
        return strcmp(start, extension.c_str()) == 0;
    }

    return false;
}

TGA readTGAArgument(int & cmdIndex, int argc, char **argv)
{
    if (cmdIndex >= argc) {
        cout << "Missing argument." << endl;
        exit(1);
    }

    if (!stringEndingWith(argv[cmdIndex], ".tga")) {
        cout << "Invalid argument, invalid file name." << endl;
        exit(1);
    }

    TGA input2 = readTGA(argv[cmdIndex],  "Invalid argument, file does not exist.");

    cmdIndex++;

    return input2;
}

int readIntegerArgument(int & cmdIndex, int argc, char **argv)
{
    if (cmdIndex >= argc) {
        cout << "Missing argument." << endl;
        exit(1);
    }

    int value = 0;
    
    try {
        value = std::stoi(argv[cmdIndex]);
    }
    catch(...) {
        cout << "Invalid argument, expected number." << endl;
        exit(1);
    }

    cmdIndex++;

    return value;
}

int main(int argc, char **argv)
{
    if (argc == 1 || (argc == 2 && !strcmp(argv[1], "--help")) ) {
        printUsage();
        return 0;
    }

    const string output = argv[1];
    
    if (!stringEndingWith(output, ".tga")) {
        cout << "Invalid file name." << endl;
        exit(1);
    }

    if (argc < 3 || !stringEndingWith(argv[2], ".tga")) {
        cout << "Invalid file name." << endl;
        exit(1);
    }

    TGA trackingImage = readTGA(argv[2], "File does not exist.");

    int cmdIndex = 3;

    do {
        
        if (cmdIndex >= argc) {
            cout << "Invalid method name." << endl;
            exit(1);
        }

        const string method = argv[cmdIndex++];

        if (method == "multiply") {

            TGA input = readTGAArgument(cmdIndex, argc, argv);
            TGA output = operationMultiply(trackingImage, input);
            trackingImage = output;

            cout << "... Multiplying" << endl;
        }
        else if (method == "subtract") {

            TGA input = readTGAArgument(cmdIndex, argc, argv);
            TGA output = operationSubtraction(trackingImage, input);
            trackingImage = output;

            cout << "... Subtracting" << endl;
        }
        else if (method == "overlay") {

            TGA input = readTGAArgument(cmdIndex, argc, argv);
            TGA output = operationOverlay(trackingImage, input);
            trackingImage = output;

            cout << "... Overlaying" << endl;
        }
        else if (method == "screen") {

            TGA input = readTGAArgument(cmdIndex, argc, argv);
            TGA output = operationScreen(input, trackingImage);
            trackingImage = output;

            cout << "... Screen" << endl;
        }
        else if (method == "combine") {

            TGA greenLayer = readTGAArgument(cmdIndex, argc, argv);
            TGA blueLayer = readTGAArgument(cmdIndex, argc, argv);
            TGA output = operationCombine(trackingImage, greenLayer, blueLayer);
            trackingImage = output;

            cout << "... Combining" << endl;
        }
        else if (method == "flip") {

            trackingImage = operationFlip(trackingImage);

            cout << "... Flipping" << endl;
        }
        else if (method == "onlyred" || method == "onlygreen" || method == "onlyblue")  {

            trackingImage = operationOnly(
                trackingImage, 
                method == "onlyred",
                method == "onlygreen", 
                method == "onlyblue"
            );

            cout << "... Operation = " << method << endl;
        }
        else if (method == "addred" || method == "addgreen" || method == "addblue") {

            int value = readIntegerArgument(cmdIndex, argc, argv);
            
            int red = 0;
            int green = 0;
            int blue = 0;
            if (method == "addred") {
                red = value;
            }
            else if (method == "addgreen") {
                green = value;
            }
            else {
                blue = value;
            }

            trackingImage = operationAddition(trackingImage, red, green, blue);

            cout << "... Operation = " << method << endl;
        }
        else if (method == "scalered" || method == "scalegreen" || method == "scaleblue") {

            int value = readIntegerArgument(cmdIndex, argc, argv);
            
            int red = 1;
            int green = 1;
            int blue = 1;
            if (method == "scalered") {
                red = value;
            }
            else if (method == "scalegreen") {
                green = value;
            }
            else {
                blue = value;
            }

            trackingImage = operationScale(trackingImage, red, green, blue);

            cout << "... Operation = " << method << endl;
        }
        else {
            cout << "Invalid method name." << endl;
            exit(1); 
        }
    }
    while (cmdIndex < argc);

    cout << "... and saving output to " << output << "!" << endl;

    writeTGA(trackingImage, output);

    return 0;
}