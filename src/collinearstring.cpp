/**
  * A class containing information about a collinear string,
  * as detected by the algrorithm described in "collinearGrouping.cpp".
  *
  * Author: phugen
  */

#include "include/collinearstring.hpp"
#include "include/auxiliary.hpp"

using namespace std;
using namespace cv;

#include <iostream>

CollinearString::CollinearString(vector<ConnectedComponent> cluster, double avgHeight)
{
    this->comps = cluster;

    // DEBUG: Init as 100 groups; find a way to make this dynamic
    // that works with the classification process in refine()
    this->groups = vector<CollinearGroup>();
    for(int i = 0; i < 100; i++)
        this->groups.push_back(CollinearGroup());

    this->phrases = vector<CollinearPhrase>();
    this->groupNo = 0;
    this->phraseNo = 0;
}

CollinearString::~CollinearString()
{

}

// Calculates the local average height from the four neighbors
// of the given component (if possible).
// Depending on the orientation of the cluster line, either
// the component's horizontal or vertical MBR lines are used
// for the calculation.
double CollinearString::localAvgHeight (vector<ConnectedComponent> cluster, int listPos)
{
    int startPos, endPos;
    double localAvg = 0;
    int dim;

    // determine which MBR dimension to use by inspecting line angle
    double angle = cluster.at(listPos).houghLine[0];

    if(angle <= 0.785398) //|| angle >= 2.53073)
        dim = 1; // line is somewhat vertical (0° - 45°), use X axis for height due to string orientation
    else
        dim = 0; // line is horizonal, use Y-axis for height

    // only one component in the cluster
    if(cluster.size() == 1)
    {
        // component is only 1 pixel wide or long
        if(cluster.at(0).mbr_max[dim] == cluster.at(0).mbr_min[dim])
            localAvg = 1;

        else
            localAvg = cluster.at(0).mbr_max[dim] - cluster.at(0).mbr_min[dim];
    }

    else
    {
        // Account for comps near the start or end of the cluster list
        listPos - 2 < 0 ? startPos = 0 : startPos = listPos - 2;
        listPos + 2 >= (int) cluster.size() ? endPos = (int) cluster.size() - 1 : endPos = listPos + 2;

        // calculate average height
        for(int i = startPos; i <= endPos; i++)
        {
            // component is only 1 pixel wide or long
            if(cluster.at(i).mbr_max[dim] == cluster.at(i).mbr_min[dim])
                localAvg += 1;

            else
                localAvg += cluster.at(i).mbr_max[dim] - cluster.at(i).mbr_min[dim];
        }

        localAvg /= endPos - startPos;
    }

    return localAvg;
}


// Calculates the distance from the component at listPos
// to its successor in the list, if possible.
double CollinearString::edgeToEdgeDistance (vector<ConnectedComponent> cluster, int listPos)
{
    // This is the last component in the list.
    // Distance should already have been calculated
    // by edgeToEdgeDistance(cluster, listPos - 1).
    if(listPos == (int) cluster.size() - 1)
        return INT_MAX; // dummy value


    // First MBR, coordinates in "matrix style", i.e. top left origin
    Vec2i mbr_min = cluster.at(listPos).mbr_min;
    Vec2i mbr_max = cluster.at(listPos).mbr_max;

    Vec2i bot_left_1 = Vec2i(mbr_max[0], mbr_min[1]);
    Vec2i bot_right_1 = mbr_max;
    Vec2i top_right_1 = Vec2i(mbr_min[0], mbr_max[1]);
    Vec2i top_left_1 = mbr_min;

    // Second MBR
    Vec2i mbr_min2 = cluster.at(listPos + 1).mbr_min;
    Vec2i mbr_max2 = cluster.at(listPos + 1).mbr_max;

    Vec2i bot_left_2 = Vec2i(mbr_max2[0], mbr_min2[1]);
    Vec2i bot_right_2 = mbr_max2;
    Vec2i top_right_2 = Vec2i(mbr_min2[0], mbr_max2[1]);
    Vec2i top_left_2 = mbr_min2;

    // define line endpoints for both MBRs
    pair<Vec2i, Vec2i> left_1 = make_pair(bot_left_1, top_left_1);
    pair<Vec2i, Vec2i> bottom_1 =  make_pair(bot_left_1, bot_right_1);
    pair<Vec2i, Vec2i> right_1 =  make_pair(bot_right_1, top_right_1);
    pair<Vec2i, Vec2i> top_1 =  make_pair(top_right_1, top_left_1);

    pair<Vec2i, Vec2i> left_2 =  make_pair(bot_left_2, top_left_2);
    pair<Vec2i, Vec2i> bottom_2 =  make_pair(bot_left_2, bot_right_2);
    pair<Vec2i, Vec2i> right_2 =  make_pair(bot_right_2, top_right_2);
    pair<Vec2i, Vec2i> top_2 =  make_pair(top_right_2, top_left_2);

    // First test if MBRs overlap - if they do, we'll consider the distance zero:
    // Check X-dimension and Y-dimension overlap.
    // Which of the two points on the line is chosen doesn't matter
    // since the checked dimension is always the same for both.
    if(!(left_1.first[1] < right_2.first[1]) && !(right_1.first[1] > left_2.first[1]) &&
            !(top_1.first[0] > bottom_2.first[0]) && !(bottom_1.first[0] < top_2.first[0]))
        return 0.;


    // The rectangles do not intersect, so find the minimum distance between the two
    // rectangles. The shortest line always has its origin on a vertex of one of
    // the two rectangles, so calculate the distances of the perpendicular line
    // segments that start at all vertices and end at each of the four sides
    // of the rectangle, then choose the minimum one out of those.
    //
    // (Could be improved by figuring out which line divides the two rectangles, so
    // the two/three "back side" lines could be culled from the calculation)
    vector<Vec2i> mbr1_points { bot_left_1, bot_right_1, top_right_1, top_left_1 };
    vector<Vec2i> mbr2_points { bot_left_2, bot_right_2, top_right_2, top_left_2 };

    vector<pair<Vec2i, Vec2i>> mbr1_sides { left_1, bottom_1, right_1, top_1 };
    vector<pair<Vec2i, Vec2i>> mbr2_sides { left_2, bottom_2, right_2, top_2 };

    double min_dist = DBL_MAX;

    // Distances from points of MBR1 to sides of MBR2
    for(auto p1 = mbr1_points.begin(); p1 != mbr1_points.end(); p1++)
        for(auto side2 = mbr2_sides.begin(); side2 != mbr2_sides.end(); side2++)
            min_dist = min(min_dist, distanceFromCartesianSegment(*p1, *side2));

    // Distances from points of MBR2 to sides of MBR1
    for(auto p2 = mbr2_points.begin(); p2 != mbr2_points.end(); p2++)
        for(auto side1 = mbr1_sides.begin(); side1 != mbr1_sides.end(); side1++)
            min_dist = min(min_dist, distanceFromCartesianSegment(*p2, *side1));

    return min_dist;
}

// Inspects the components in this string and classifies them as
// isolated, part of words or as part of phrases in order to
// refine the selection of components to delete from the image.
void CollinearString::refine ()
{
    // Single, isolated component
    if(this->comps.size() == 1)
    {
        CollinearGroup gr;
        gr.type = 'i';
        gr.chars.push_back(comps.at(0));
        this->groups.push_back(gr);

        CollinearPhrase phr;
        phr.words.push_back(gr);
        this->phrases.push_back(phr);

        groupNo = 1;
        phraseNo = 1;

        return;
    }

    double tc, tw; // character and word thresholds
    double distToNext; // distance to next component

    int listpos = 0; // position of current component in the list

    int oldGroupNumber = -1; // for checking if the group count advanced
    int groupNumber = 0; // current word count

    int oldPhraseNumber = -1; // for checking if the phrase count advanced
    int phraseNumber = 0; // current phrase count

    // 2.) Check inter-character distances until
    // a component doesn't satisfy the threshold.
    for(auto comp = this->comps.begin(); comp != this->comps.end() - 1; comp++)
    {
        // add new group if needed
        /*if(groupNumber != oldGroupNumber)
        {
            this->groups.push_back(CollinearGroup());
            oldGroupNumber = groupNumber;
        }*/

        // add new phrase if needed
        if(phraseNumber != oldPhraseNumber)
        {
            this->phrases.push_back(CollinearPhrase());
            oldPhraseNumber = phraseNumber;
        }

        listpos = comp - comps.begin(); // get current position in component list
        CollinearGroup* curGroup = &this->groups.at(groupNumber); // shorthand ref

        // calculate inter-character and inter-word thresholds
        tc = localAvgHeight(this->comps, listpos);
        tw = 2.5 * tc;

        // calculate distance from current component to the next
        // in the component list
        distToNext = edgeToEdgeDistance(this->comps, listpos);

        // include current character in current group
        // if the group is new
        if(curGroup->size() == 0)
            curGroup->chars.push_back(this->comps.at(listpos)); // head

        // If the distance to the next component is below the
        // inter-character threshold "tc", then components
        // i and i+1 are part of the same word
        if(distToNext <= tc)
        {
            curGroup->chars.push_back(this->comps.at(listpos + 1)); // tail

            // if the word doesn't belong to a phrase already,
            // mark it as a word
            if(curGroup->type != 'p')
                curGroup->type = 'w';
        }

        // current word ends because inter-character distance is too high
        else
        {
            groupNumber++;

            // 3.) If the distance to the next component is below the
            // inter-word threshold "tw", then the previous word and the
            // current component belong to the same phrase.
            CollinearPhrase* curPhrase = &(this->phrases.at(phraseNumber));
            CollinearGroup* prevGroup = curGroup;
            curGroup = &this->groups.at(groupNumber);

            if(distToNext < tw)
            {
                // mark previous and current group as part of a phrase
                prevGroup->type = 'p';
                curGroup->type = 'p';

                // make both part of the current phrase
                curPhrase->words.push_back(*prevGroup);
                curPhrase->words.push_back(*curGroup);
            }

            // The current component is not part of the current phrase,
            // start a new one instead.
            else
                phraseNumber++;
        }
    }

    // there is only one phrase, so link the current
    // groups to it
    if(phraseNumber == 0)
    {
        for(auto grp = this->groups.begin(); grp != this->groups.end(); grp++)
        {
            (*grp).type = 'p';
            this->phrases.at(0).words.push_back(*grp);
        }
    }


    // 4.) Search for consecutive isolated groups in phrases. If there are
    // cases in which there are two or more consecutive isolated groups in
    // a phrase, either truncate or split the groups.
    for(int p = 0; p < phraseNumber; p++)
        for(int g = 0; g < (int) this->phrases.at(p).words.size() - 1; g++)
        {
            CollinearPhrase* cp = &(this->phrases.at(p));

            // find consecutive isolated groups
            // in the current phrase
            if(cp->words.at(g).type == 'i' && cp->words.at(g + 1).type == 'i')
            {
                for(int c = g; c < (int) cp->words.size(); c++)
                {
                    if(cp->words.at(c).type != 'i')
                        break;

                    cout << "ISOLATED COMPONENT AT POS " << c << "IN PHRASE " << p << "\n";
                }

            }
        }


    this->groupNo = groupNumber + 1;
    this->phraseNo = phraseNumber + 1;
}
