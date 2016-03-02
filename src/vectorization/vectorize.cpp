/**
  * Implements a raster-to-vector graphic transformation
  * and outputs the vector graphic as an .svg.
  *
  * Author: phugen
  */

#include "include/vectorization/vectorize.hpp"
#include "include/cairo/cairo.h"
#include "include/cairo/cairo-svg.h"
#include "include/vectorization/zhangsuen.hpp"
#include "include/vectorization/moore.hpp"
#include "include/vectorization/douglaspeucker.h"


#include <iostream>
#include <stack>

using namespace std;
using namespace cv;


// Create a .svg file "filename.svg" based on the extracted vector lines.
void vectorsToFile (Mat* thinned, Mat* original_image, vector<vector<pixel*>> paths, string filename)
{
    namedWindow("before", WINDOW_NORMAL);
    imshow("before", *thinned);

    // debug: overlay found lines on image in blue
    Mat vectoronly = Mat(thinned->rows, thinned->cols, thinned->type());
    cvtColor(*thinned, vectoronly, CV_GRAY2RGB);


    for(auto path = paths.begin(); path != paths.end(); path++)
    {
        for(auto p = (*path).begin(); p != (*path).end() - 1; p++)
        {
            pixel* start = *p;
            pixel* end = *(p+1);

            Point pt1 = Point(start->coord[1], start->coord[0]);
            Point pt2 = Point(end->coord[1], end->coord[0]);

            line(vectoronly, pt1, pt2, Scalar(255, 0, 0), 1);
        }
    }

    namedWindow("VECTORS", WINDOW_NORMAL);
    imshow("VECTORS", vectoronly);

    // Set up cairo canvas
    cairo_surface_t *surface;
    cairo_t *cr;
    surface = cairo_svg_surface_create((filename + ".svg").c_str(), thinned->cols, thinned->rows);
    cr = cairo_create(surface);
    cairo_set_source_rgb(cr, 0, 0, 0); // set background color
    cairo_set_line_width(cr, 1); // set line width
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_SQUARE); // don't smoothen line ends

    // do a rough coloring for presentation purposes
    for(int i = 0; i < original_image->rows; i++)
        for(int j = 0; j < original_image->cols; j++)
        {
            if( (i % 5) == 0 && (j % 5) == 0)
            {
                Vec3b pxclr = original_image->at<Vec3b>(i, j);

                // cairo needs floats in [0, 1]
                float red = pxclr[2];
                red /= 255;

                float green = pxclr[1];
                green /= 255;

                float blue = pxclr[0];
                blue /= 255;

                // pick (interpolated?) color from original image pixel
                // and color background with vector rectangles

                cairo_set_source_rgb (cr, red, green, blue);
                cout << pxclr << "\n";
                cairo_rectangle(cr, j-0.5, i-0.5, 1, 1);
                cairo_fill (cr);
            }
        }

    // reset color to black
    cairo_set_source_rgb (cr, 0, 0, 0);

    // Write line segment descriptions to .svg file
    for(auto path = paths.begin(); path != paths.end(); path++)
    {
        for(auto p = (*path).begin(); p != (*path).end() - 1; p++)
        {
            pixel* start = *p;
            pixel* end = *(p+1);

            // cairo can't handle 1px paths with square line caps
            // so draw a 1px square instead!
            if(start->coord == end->coord)
            {
                // use "half" pixels to get squares that don't stretch
                // over the border between two pixels if coordinate is odd
                //if(start->coord[1] % 2 == 1)
                    cairo_rectangle(cr, start->coord[1]-0.5, start->coord[0]-0.5, 1, 1);

                //else
                //    cairo_rectangle(cr, start->coord[1], start->coord[0], 1, 1);

                cairo_fill (cr);
            }

            // draw normal line
            else
            {
                cairo_move_to(cr, start->coord[1], start->coord[0]);
                cairo_line_to(cr, end->coord[1], end->coord[0]);
                cairo_stroke(cr);
            }
        }
    }


    // Free canvas
    cairo_surface_destroy(surface);
    cairo_destroy(cr);
}



// Traverse graph until all paths in the graph
// are found.
vector<vector<vectorLine*>> lineDFS (set<vectorLine*>* lines)
{
    vector<vector<vectorLine*>> local_paths; // holds paths found in current graph
    stack<vectorLine*> found; // nodes that still have to be walked
    set<vectorLine*> closed; // set of nodes that have been walked already
    vector<vectorLine*> currentPath; // contains nodes that are in the current subpath

    found.push((*lines->begin()));

    // keep going until all nodes in this subgraph were found
    while(found.size() != 0)
    {
        vectorLine* current = found.top();
        found.pop();
        closed.insert(current);

        // find all nodes that share the same nodes as this node's line
        // (except for ones that have been found already)
        set<vectorLine*> connected;

        // find connecting lines
        // (CHANGE TO FIXED MAP!)
        for(auto l = lines->begin(); l != lines->end(); l++)
        {
            // if the lines shares an endpoint with current line
            if(current->getEnd() == (*l)->getStart() ||
               current->getEnd() == (*l)->getEnd())
            {
                // if the line hasn't been found already
                if(closed.find(*l) == closed.end())
                    connected.insert(*l);
            }
        }

        vector<vectorLine*> path;

        // current endpoint is a true node
        if(connected.size() == 0 ||
                connected.size() > 1)
        {
            // add current line
            currentPath.push_back(current);

            // delete local path from global line set
            for(auto l = currentPath.begin(); l != currentPath.end(); l++)
            {
                path.push_back(*l);
                lines->erase(*l);
            }

            // add to local path list
            local_paths.push_back(path);

            // reset current path
            currentPath.clear();
        }

        // otherwise, extend this path
        else
        {
            currentPath.push_back(current);
        }

        // add connected lines to the found set
        for(auto l = connected.begin(); l != connected.end(); l++)
            found.push(*l);
    }

    return local_paths;
}


// walk image from top to bottom, left to right
// fuse nodes with predecessor node to get shared nodes
// between lines
void fuseNodes(Mat image, vector<pixel*> pixels, multimap<pixel*, vectorLine*>* nodeToLine)
{
    int height = image.rows;
    int width = image.cols;

    // Walk image
    for(int i = 0; i < image.rows; i++)
    {
        for(int j = 0; j < image.cols; j++)
        {
            pixel* current = pixels.at(i * width + j);
            vectorLine* line = current->line;

            // if current pixel is a node
            if(nodeToLine->find(current) != nodeToLine->end())
            {
                // find predecessor pixels
                // order of inspection: top->bottom, left->right
                vector<pixel*> toCheck;

                // top left pixel
                if(i == 0 && j == 0)
                {
                    // none, first pixel in image
                }

                // top right pixel
                else if(i == 0 && j == width - 1)
                {
                    toCheck.push_back(pixels.at(i * width + ((j-1)))); // w
                }


                // bottom left pixel
                else if (i == height && j == 0)
                {
                    toCheck.push_back(pixels.at((i-1) * width + j)); // n
                    toCheck.push_back(pixels.at(((i-1)) * width + (j+1))); // ne
                }

                // bottom right pixel
                else if (i == height && j == width - 1)
                {
                    toCheck.push_back(pixels.at((i-1) * width + ((j-1)))); // nw
                    toCheck.push_back(pixels.at((i-1) * width + j)); // n
                    toCheck.push_back(pixels.at(i * width + ((j-1)))); // w
                }

                // upper border
                else if (i == 0)
                {
                    toCheck.push_back(pixels.at(i * width + ((j-1)))); // w
                }

                // left border
                else if (j == 0)
                {
                    toCheck.push_back(pixels.at((i-1) * width + j)); // n
                    toCheck.push_back(pixels.at(((i-1)) * width + (j+1))); // ne
                }


                // right border
                else if (j == width - 1)
                {
                    toCheck.push_back(pixels.at((i-1) * width + ((j-1)))); // nw
                    toCheck.push_back(pixels.at((i-1) * width + j)); // n
                    toCheck.push_back(pixels.at(i * width + ((j-1)))); // w
                }

                // lower border
                else if (i == height - 1)
                {
                    toCheck.push_back(pixels.at((i-1) * width + ((j-1)))); // nw
                    toCheck.push_back(pixels.at((i-1) * width + j)); // n
                    toCheck.push_back(pixels.at(((i-1)) * width + (j+1))); // ne
                    toCheck.push_back(pixels.at(i * width + ((j-1)))); // w
                }

                // Normal case - pixel is somewhere
                // in the image, but not near any border.
                else
                {
                    toCheck.push_back(pixels.at((i-1) * width + ((j-1)))); // nw
                    toCheck.push_back(pixels.at((i-1) * width + j)); // n
                    toCheck.push_back(pixels.at(((i-1)) * width + (j+1))); // ne
                    toCheck.push_back(pixels.at(i * width + ((j-1)))); // w
                }

                // filter non-node pixels
                vector<pixel*> nodes;

                for(auto px = toCheck.begin(); px != toCheck.end(); px++)
                {
                    if(nodeToLine->find(*px) != nodeToLine->end())
                        nodes.push_back(*px);
                }

                // coordinates for predecessor coordinates
                int i_nw = i-1;
                int j_nw = j-1;

                int i_n = i-1;
                int j_n = j;

                int i_ne = i-1;
                int j_ne = j+1;

                int i_w = i;
                int j_w = j-1;

                // if there is only one
                // predecessor node
                if(nodes.size() == 1)
                {
                    // current is a start node
                    if(current == current->line->getStart())
                    {
                        // update mapping
                        if(line->getStart() != line->getEnd()) // respect 1px lines
                            nodeToLine->erase(line->getStart());

                        nodeToLine->insert(make_pair(nodes.at(0), line));

                        // set new start
                        current->line->setStart(nodes.at(0));
                    }

                    // current is an end node
                    else if(current == current->line->getEnd())
                    {
                        if(line->getStart() != line->getEnd())
                            nodeToLine->erase(nodes.at(0));

                        nodeToLine->insert(make_pair(current, (nodes.at(0))->line));
                        nodes.at(0)->line->setEnd(current);
                    }
                }

                else if(nodes.size() == 2)
                {
                    pixel* first = nodes.at(0);
                    pixel* second = nodes.at(1);

                    // pred nodes are NW and N
                    if(first->coord == Vec2i(i_nw, j_nw) &&
                       second->coord == Vec2i(i_n, j_n))
                    {
                        // update mapping
                        if(line->getStart() != line->getEnd())
                            nodeToLine->erase(line->getStart());

                        nodeToLine->insert(make_pair(second, line));

                        // set new start
                        line->setStart(second); // start = N
                    }

                    // pred nodes are N and NE
                    else if(first->coord == Vec2i(i_n, j_n) &&
                       second->coord == Vec2i(i_ne, j_ne))
                    {
                        if(line->getStart() != line->getEnd())
                            nodeToLine->erase(line->getStart());

                        nodeToLine->insert(make_pair(first, line));

                        line->setStart(first); // start = N
                    }


                    // pred nodes are NW and W
                    else if(first->coord == Vec2i(i_nw, j_nw) &&
                            second->coord == Vec2i(i_w, j_w))
                    {
                        if(line->getStart() != line->getEnd())
                            nodeToLine->erase(line->getStart());

                        nodeToLine->insert(make_pair(second, line));

                        line->setStart(second); // start = W
                    }

                    // pred nodes are N and W ("double fuse" case - two lines end in current)
                    else if(first->coord == Vec2i(i_n, j_n) &&
                            second->coord == Vec2i(i_w, j_w))
                    {
                        nodeToLine->erase(first->line->getStart());
                        nodeToLine->erase(second->line->getEnd());
                        nodeToLine->insert(make_pair(current, first->line));
                        nodeToLine->insert(make_pair(current, second->line));

                        first->line->setStart(current);
                        second->line->setEnd(current);
                    }

                }

                else if (nodes.size() == 3)
                {
                    pixel* first = nodes.at(0);
                    pixel* second = nodes.at(1);
                    pixel* third = nodes.at(2);

                    // pred nodes are NW, N, NE
                    if(first->coord == Vec2i(i_nw, j_nw) &&
                       second->coord == Vec2i(i_n, j_n) &&
                       third->coord == Vec2i(i_ne, j_ne))
                    {
                        nodeToLine->erase(line->getStart());
                        nodeToLine->insert(make_pair(second, line));

                        line->setStart(second); // start = N
                    }
                }


                else if (nodes.size() == 4)
                {
                    // needed? possible?
                }
            }
        }
    }
}


// Retrieves all subpaths in the extracted vector graphs
// and simplifies them.
vector<vector<pixel*>> refineVectors (Mat* image, multimap<pixel*, vectorLine*>* nodeToLine,
                                      vector<pixel*>* pixels, double epsilon)
{
    pixel* dummy = new pixel(Vec2i(-1, -1), NULL, false);

    set<vectorLine*> lines;
    for(auto px = nodeToLine->begin(); px != nodeToLine->end(); px++)
    {
        lines.insert((*px).second);
    }

    // fuse adjacent nodes in all graphs
    fuseNodes(*image, *pixels, nodeToLine);


    // Walk through all nodes via DFS and
    // find sub-paths that can be refined.
    //
    // Since graphs are usually not made up of
    // only one component, this implies that the
    // DFS needs to be applied as long as there are
    // nodes that are still undiscovered.
    //
    // A path's start and end nodes are "true" nodes, as defined in
    // the Moore paper. True nodes are those nodes which
    // connect to exactly one line or more than two lines.
    //
    // Once a true end node is reached, the path so far is recorded as
    // a set of nodes, and the algorithm continues.

    vector<vector<vectorLine*>> allPaths; // all subpaths of all graphs

    // traverse all graphs
    while(lines.size() != 0)
    {
        // recursively walk graph and find paths
        // (lines are deleted from global line set inside the function)
        vector<vector<vectorLine*>> subPaths = lineDFS(&lines);

        // add sub paths to list of globally found paths
        for(auto path = subPaths.begin(); path != subPaths.end(); path++)
            allPaths.push_back(*path);
    }

    // Extract individual nodes of each path for use in
    // the Douglas-Peucker algorithm
    vector<vector<pixel*>> pathsAsNodes;

    for(auto path = allPaths.begin(); path != allPaths.end(); path++)
    {
        vector<pixel*> pathNodes;
        //pathNodes.push_back((*path).at(0)->getStart()); // add first node

        // get end nodes (avoid duplicates)
        for(int i = 0; i < (int) ((*path).size()); i++)
        {
            pathNodes.push_back((*path).at(i)->getStart());
            pathNodes.push_back((*path).at(i)->getEnd());
        }

        pathsAsNodes.push_back(pathNodes);
    }

    // simplify paths via Douglas-Peucker algorithm
    for(auto path = pathsAsNodes.begin(); path != pathsAsNodes.end(); path++)
    {
        *path = douglasPeucker(*path, epsilon);
    }

    delete dummy;
    // delete pixels;
    // delete original lines?

    return pathsAsNodes;
}


// Initialize all pixels with their coordinates
// in the picture.
void initPixels(vector<pixel*>* pixels, Mat* image)
{
    for(int i = 0; i < image->rows; i++)
    {
        for(int j = 0; j < image->cols; j++)
        {
            pixels->at(i * image->cols + j) = new pixel(Vec2i(i, j), NULL, false);
        }
    }
}

// Expects binary image (blackLayer)
void vectorizeImage (Mat* blacklayer, Mat* original_image, string filename, double epsilon)
{
    cout << "\n ------------------------- \n";
    cout << "Starting vectorization ...\n";

    multimap<pixel*, vectorLine*> nodeToLine; // provides a endpoint->line mapping
    vector<vector<pixel*>> refinedPaths; // holds results of douglas-peucker algorithm

    vector<pixel*>* pixels = new vector<pixel*>((blacklayer->rows + 2) * (blacklayer->cols + 2)); // states of all pixels (+ dummy values for 1px border)
    pixel* dummy = new pixel(Vec2i(-1, -1), NULL, false);
    initPixels(pixels, blacklayer);

    Mat inverted = Mat(blacklayer->rows, blacklayer->cols, blacklayer->type());
    Mat thinned = Mat(blacklayer->rows, blacklayer->cols, blacklayer->type());

    // thin image using Zhang-Suen
    cout << "Extracting image skeleton... \n";
    bitwise_not(*blacklayer, inverted);
    thinning(inverted);
    bitwise_not(inverted, thinned);

    imwrite("thinned.png", thinned);

    // create vector lines
    cout << "Extracting vectors from raster image... \n";
    nodeToLine = mooreVector(thinned, pixels, dummy);

    // refine vectors by removing unnecessary nodes
    cout << "Refining vector data... \n";
    refinedPaths = refineVectors(blacklayer, &nodeToLine, pixels, epsilon);

    // recover image topology and add color
    cout << "Recovering image topology... \n";
    // recoverTopology(lines);

    // write vector lines to file
    cout << "Writing vector data to file << " << filename << ".svg...\n";
    vectorsToFile(&thinned, original_image, refinedPaths, filename);

    // delete allocated line objects
    cout << "Cleaning up... \n";
    /*for(auto l = refinedLines.begin(); l != refinedLines.end(); l++)
        delete (*l);

    for(auto l = nodeToLine.begin(); l != nodeToLine.end(); l++)
        delete (*l).second;

    // deallocate pixel objects
    for(auto l = nodeToLine.begin(); l != nodeToLine.end(); l++)
        delete (*l).first;*/

    cout << "VECTORIZATION DONE!\n";
}



