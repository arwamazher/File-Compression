/*
 * STARTER CODE: util.h
 *
 * PROGRAM: File Compression Project
 *
 * DESCRIPTION: This project builds a file compression algorithm using binary trees and priority queues.
 * The user will be able to compress and decompress files using the standard Huffman algorithm for encoding
 * and decoding, along with a provided custom hashmap class.
 *
 * CLASS: CS 251 | 9am Lecture | Wednesday 10am Lab
 * SYSTEM USED: Windows using CLion
 * AUTHOR: Arwa Mazher
 */

#pragma once
#include <queue>

typedef hashmap hashmapF;
typedef unordered_map <int, string> hashmapE;

struct HuffmanNode {
    int character;
    int count;
    HuffmanNode* zero;
    HuffmanNode* one;
};

struct compare
{
    bool operator()(const HuffmanNode *lhs,
        const HuffmanNode *rhs)
    {
        return lhs->count > rhs->count;
    }
};

//
// *This method frees the memory allocated for the Huffman tree.
//
void freeTree(HuffmanNode* node) {
    if (node == nullptr) {
        return;
    }
    else {
        freeTree(node->zero);
        freeTree(node->one);
        node->zero = nullptr;
        node->one = nullptr;
        delete node;
    }
    
}

//
// *This function build the frequency map.  If isFile is true, then it reads
// from filename.  If isFile is false, then it reads from a string filename.
//
void buildFrequencyMap(string filename, bool isFile, hashmapF &map) {
    char c;
    // read from file
    if (isFile) {
        ifstream inFS(filename);
        while (inFS.get(c)) {
            // if in the map, incrememnt value
            if (map.containsKey(int(c))) {
                map.put(int(c), map.get(int(c))+1);
            }
            // not in map, first occurrence = 1
            else {
                map.put(int(c), 1);
            }
        }
    }

    // read from filename
    else {
        for (char c : filename) {
            // if in the map, incrememnt value
            if (map.containsKey(int(c))) {
                map.put(int(c), map.get(int(c))+1);
            }
            // not in map, first occurrence = 1
            else {
                map.put(int(c), 1);
            }
        }
    }
    // newline character
    map.put(256, 1);
 }

//
// *This function builds an encoding tree from the frequency map.
//
HuffmanNode* buildEncodingTree(hashmapF &map) {
    priority_queue<HuffmanNode*, vector<HuffmanNode*>, compare> pq;

    // loop through map to make HuffmanNode*s and add them to pq
    vector<int> keys = map.keys();
    for (int i = 0; i < keys.size(); i++) {
        HuffmanNode* addNode = new HuffmanNode;
        addNode->character = keys[i];
        addNode->count = map.get(keys[i]);
        addNode->zero = nullptr;
        addNode->one = nullptr;
        pq.push(addNode);
    }

    // loop through pq and connect nodes to the sum
    while (pq.size() > 1) {
        HuffmanNode* tempFirst = pq.top();
        pq.pop();
        if (pq.top() != nullptr) {
            HuffmanNode* tempSecond = pq.top();
            pq.pop();
            HuffmanNode* sumNode = new HuffmanNode;
            sumNode->count = tempFirst->count + tempSecond->count;
            sumNode->character = 257;
            sumNode->zero = tempFirst;
            sumNode->one = tempSecond;
            pq.push(sumNode);
        }
    }
    
    return pq.top();
}

//
// *Recursive helper function for building the encoding map.
//
void _buildEncodingMap(HuffmanNode* node, hashmapE &encodingMap, string str,
                       HuffmanNode* prev) {
    // base case: leaf node
    if (node->character != 257) {
        encodingMap.emplace(node->character, str);
        return;
    }

    else {
        str += "0";
        _buildEncodingMap(node->zero, encodingMap, str, prev);
        str = str.substr(0, str.size()-1);
        str += "1";
        _buildEncodingMap(node->one, encodingMap, str, prev);
        str = str.substr(0, str.size()-1);
    }
    
}

//
// *This function builds the encoding map from an encoding tree.
//
hashmapE buildEncodingMap(HuffmanNode* tree) {
    hashmapE encodingMap;
    string str = "";
    if (tree != nullptr) {
        _buildEncodingMap(tree, encodingMap, str, nullptr);
    }
    
    return encodingMap;
}

//
// *This function encodes the data in the input stream into the output stream
// using the encodingMap.  This function calculates the number of bits
// written to the output stream and sets result to the size parameter, which is
// passed by reference.  This function also returns a string representation of
// the output file, which is particularly useful for testing.
//
string encode(ifstream& input, hashmapE &encodingMap, ofbitstream& output,
              int &size, bool makeFile) {
    string str = "";
    char c;
    // loop through input and add corresponding values based on encodingMap
    while (input.get(c)) {
        str += encodingMap[int(c)];
    }
    // add PSEUDO_EOF character to end of str
    str += encodingMap[PSEUDO_EOF];

    // if makeFile is true, write to output file
    if (makeFile) {
        for (char ch : str) {
            if (ch == '0') {
                output.writeBit(0);
            }
            else {
                output.writeBit(1);
            }
            size++;
        }
    }
    
    return str;
}


//
// *This function decodes the input stream and writes the result to the output
// stream using the encodingTree.  This function also returns a string
// representation of the output file, which is particularly useful for testing.
//
string decode(ifbitstream &input, HuffmanNode* encodingTree, ofstream &output) {
    HuffmanNode* root = encodingTree;
    string str = "";
    while (!input.eof()) {
        int bit = input.readBit();

        // if bit is 1, go right
        if (bit == 1) {
            encodingTree = encodingTree->one;
        }
        // if bit is 0, go left
        if (bit == 0) {
            encodingTree = encodingTree->zero;
        }
        // if reach a leaf, add char to output and reset to root
        if (encodingTree->character != NOT_A_CHAR && encodingTree->character != PSEUDO_EOF) {
            char c = encodingTree->character;
            str += c;
            encodingTree = root;    // not sure?
        }
        // if reach EOF, break
        if (encodingTree->character == PSEUDO_EOF) {
            break;
        }
    }

    // put each char into output
    for (char ch : str) {
        output.put(ch);
    }

    return str;
}

//
// *This function completes the entire compression process.  Given a file,
// filename, this function (1) builds a frequency map; (2) builds an encoding
// tree; (3) builds an encoding map; (4) encodes the file (don't forget to
// include the frequency map in the header of the output file).  This function
// should create a compressed file named (filename + ".huf") and should also
// return a string version of the bit pattern.
//
string compress(string filename) {
    // create frequencyMap
    hashmapF frequencyMap;
    buildFrequencyMap(filename, true, frequencyMap);
    // create encoding tree
    HuffmanNode* tree = buildEncodingTree(frequencyMap);
    // create encodingMap
    hashmapE encodingMap = buildEncodingMap(tree);

    // add .huf to compressed filename
    string newFile = filename + ".huf";
    ofbitstream output(newFile);
    output << frequencyMap;

    ifstream input(filename);
    int size;
    string str = encode(input, encodingMap, output, size, true);
    freeTree(tree);
    return str;
}

//
// *This function completes the entire decompression process.  Given the file,
// filename (which should end with ".huf"), (1) extract the header and build
// the frequency map; (2) build an encoding tree from the frequency map; (3)
// using the encoding tree to decode the file.  This function should create a
// compressed file using the following convention.
// If filename = "example.txt.huf", then the uncompressed file should be named
// "example_unc.txt".  The function should return a string version of the
// uncompressed file.  Note: this function should reverse what the compress
// function did.
//
string decompress(string filename) {
    ifbitstream input(filename);

    // parse filename
    string newFile = filename;
    int find = newFile.find(".txt.huf");
    newFile = newFile.substr(0, find);
    newFile += "_unc.txt";
    ofstream output(newFile);

    // create frequencyMap
    hashmapF frequencyMap;
    input >> frequencyMap;
    // create encoding tree
    HuffmanNode* tree = buildEncodingTree(frequencyMap);

    string str = decode(input, tree, output);
    freeTree(tree);
    return str;
}