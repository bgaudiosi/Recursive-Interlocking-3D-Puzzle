/**
    CS591-W1 Final Project
    ExtractPartitions.cpp
    Purpose: For generating the partitions of a given voxel list

    @author Ben Gaudiosi
    @version 1.0 5/01/2018 
*/
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <list>
#include <limits>
#include "../include/ExtractPartitions.h"
#include "../include/CompFab.h"

bool debug = false;

/**
    Blank constructor for the Voxel class. Generates a voxel at the origin.
*/
Voxel::Voxel() {
    x = 0;
    y = 0;
    z = 0;
}

/** 
    Constructor for the Voxel class.

    @param x The x coordinate of the voxel.
    @param y The y coordinate of the voxel.
    @param z The z coordinate of the voxel.
*/
Voxel::Voxel(int x_val, int y_val, int z_val) {
    x = x_val;
    y = y_val;
    z = z_val;
}

/**
    Constructor for the VoxelPair class.

    @param blockerVox The blocker voxel.
    @param blockerScore The accessibility score of the blocker voxel.
    @param blockeeVox The blockee voxel.
    @param blockeeScore The accessibility score of the blockee voxel.

*/
VoxelPair::VoxelPair(Voxel blockerVox, double blockerScore, Voxel blockeeVox, double blockeeScore) {
    blocker = blockerVox;
    blocker_accessibility = blockerScore;
    blockee = blockeeVox;
    blockee_accessibility = blockeeScore;
}

/**
    Constructor for the VoxelSort class.

    @param vox The voxel to be sorted.
    @param totalScore The score of the voxel in this class.
*/
VoxelSort::VoxelSort(Voxel vox, double totalScore) {
    voxel = vox;
    score = totalScore;
}

/**
    String representation of the voxel class.

    @return A human readable string of a voxel as (x, y, z).
*/
std::string Voxel::toString() {
    std::string vox("(" + std::to_string(x) + "," + std::to_string(y) + "," + std::to_string(z) + ")");
    return vox;
}

/**
    String representation of the VoxelPair class.

    @return A human readable string of a VoxelPair as (x1, y1, z1): score, (x2, y2, z2): score
*/
std::string VoxelPair::toString() {
    return "(" + blocker.toString() + ":" + std::to_string(blocker_accessibility)  + " ," + blockee.toString() + ": " + std::to_string(blockee_accessibility) + ")";
}

/**
    Constructor for the AccessibilityStruct class.
    
    @param lowerLeft A vector representing the lower left voxel in the grid. Usually is (0,0,0).
    @param dimX The dimension of x.
    @param dimY The dimension of y.
    @param dimZ The dimension of z.
*/
AccessibilityStruct::AccessibilityStruct(CompFab::Vec3 lowerLeft, unsigned int dimX, unsigned int dimY, unsigned int dimZ) {
    m_lowerLeft = lowerLeft;
    m_dimX = dimX;
    m_dimY = dimY;
    m_dimZ = dimZ;
    m_size = dimX*dimY*dimZ;

    m_scoreArray = new double[m_size];

    for(unsigned int i=0; i<m_size; ++i)
    {
        m_scoreArray[i] = 0.0;
    }

}

/**
    Destructor for the AccessibilityStruct class.
*/
AccessibilityStruct::~AccessibilityStruct()
{
    delete[] m_scoreArray;
}

/**
    Prints a list of voxels.

    @param list The list of voxels to be printed.
*/
void printList(std::vector<Voxel> list) {
    for (int i = 0; i < list.size(); i++) {
        std::cout << "\t\t" << list[i].toString() << std::endl;
    }
}

/**
    Finds seeds for the key piece to start from.

    @param voxel_list A VoxelGrid from which to generate the puzzle.
    @return A list of potential seeds for the key piece
*/
std::vector<Voxel> findSeeds( CompFab::VoxelGrid * voxel_list ) {
    std::vector<Voxel> seeds;

    int nx = voxel_list->m_dimX;
    int ny = voxel_list->m_dimY;
    int nz = voxel_list->m_dimZ;
    
    int above = 0;
    int one_side_adjacent;
    for (int i = 0; i < nx; i++) {
        for (int j = 0; j < ny; j++) {
            for (int k = 0; k < nz; k++) {
                if (voxel_list->isInside(i,j,k) == 1) {
                    // First check if any voxels above
                    for (int z = k+1 ; z < nz; z++) {
                        if (voxel_list->isInside(i,j,z) == 1) {
                            above = 1;
                            break;
                        }
                    }
                    if (above) {
                        above = 0;
                        continue;
                    }
                    //Now, check that only two other faces are open and (except bottom)
                    one_side_adjacent = 0;

                    if ( i != 0 ) {
                        if (voxel_list->isInside(i-1,j,k) == 1) {
                           one_side_adjacent++; 
                        } 
                    }
                    if ( i != nx-1 ) {
                        if (voxel_list->isInside(i+1,j,k) == 1) {
                            one_side_adjacent++;
                        }
                    }
                    if (j != 0) {
                        if (voxel_list->isInside(i,j-1,k) == 1) {
                            one_side_adjacent++;  
                        }
                    }
                    if (j != ny-1) {
                        if (voxel_list->isInside(i,j+1,k) == 1) {
                            one_side_adjacent++;
                        }
                    }
                    if (one_side_adjacent == 3) {
                        seeds.push_back(Voxel(i, j, k)); 
                    } 
                }
            }
        }
    }
    return seeds;
}

/**
    Finds the neighbors of a voxel.

    @param voxel The voxel from which to find neighbors.
    @param voxel_list A VoxelGrid representing the current state of the puzzle.
    @return The neighbors of the voxel.
*/
std::vector<Voxel> getNeighbors(Voxel voxel, CompFab::VoxelGrid * voxel_list, int pieceId) {
    std::vector<Voxel> neighbors;
    int nx = voxel_list->m_dimX;
    int ny = voxel_list->m_dimY;
    int nz = voxel_list->m_dimZ;
    
    if ( voxel.x != 0 ) {
        if (voxel_list->isInside(voxel.x-1,voxel.y,voxel.z) == pieceId) {
            neighbors.push_back(Voxel(voxel.x-1,voxel.y,voxel.z));
        }
    }   
    if ( voxel.x != nx-1 ) {
        if (voxel_list->isInside(voxel.x+1,voxel.y,voxel.z) == pieceId) {
            neighbors.push_back(Voxel(voxel.x+1,voxel.y,voxel.z));
        }
    }   
    if (voxel.y != 0) {
        if (voxel_list->isInside(voxel.x,voxel.y-1,voxel.z) == pieceId) {
            neighbors.push_back(Voxel(voxel.x,voxel.y-1,voxel.z));
        }
    }   
    if (voxel.y != ny-1) {
        if (voxel_list->isInside(voxel.x,voxel.y+1,voxel.z) == pieceId) {
            neighbors.push_back(Voxel(voxel.x,voxel.y+1,voxel.z));
        }
    }   
    if (voxel.z != 0) {
        if (voxel_list->isInside(voxel.x,voxel.y,voxel.z-1) == pieceId) {
            neighbors.push_back(Voxel(voxel.x,voxel.y,voxel.z-1));
        }
    }   
    if (voxel.z != nz-1) {
        if (voxel_list->isInside(voxel.x,voxel.y,voxel.z+1) == pieceId) {
            neighbors.push_back(Voxel(voxel.x,voxel.y,voxel.z+1));
        }
    }
    return neighbors;
}

/**
    Finds the accessibility scores of a VoxelGrid.

    @param voxel_list A VoxelGrid representing the current state of the puzzle.
    @param alpha A double used as a param for how to generate the scores.
    @param recurse How many layers of recursion to perform in score generation.
    @param pieceId The id of the piece we are scoring. Is usually 1.
    @return The accessiblity scores of each voxel in the form of an AccessibilityGrid.
*/
AccessibilityGrid * accessibilityScores( CompFab::VoxelGrid * voxel_list, double alpha, unsigned int recurse, int pieceId) {
    int nx = voxel_list->m_dimX;
    int ny = voxel_list->m_dimY;
    int nz = voxel_list->m_dimZ;
    CompFab::Vec3 start = CompFab::Vec3(0.0, 0.0, 0.0);
    AccessibilityGrid * scores = new AccessibilityGrid(start, nx, ny, nz);
    if (recurse == 0) {
        for (int i = 0; i < nx; i++) {
            for (int j = 0; j < ny; j++) {
                for (int k = 0; k < nz; k++) {
                    scores->score(i, j, k) = getNeighbors(Voxel(i, j, k), voxel_list, pieceId).size();         
                }
            }
        }
    } else {
        double current_score;
        double multiplier = pow(alpha, recurse);
        std::vector<Voxel> neighbors;
        AccessibilityGrid * old_scores = accessibilityScores( voxel_list, alpha, recurse-1, pieceId);
        for (int i = 0; i < nx; i++) {
            for (int j = 0; j < ny; j++) {
                for (int k = 0; k < nz; k++) {
                    neighbors = getNeighbors(Voxel(i, j, k), voxel_list, pieceId);
                    current_score = 0;
                    for (int n = 0; n < neighbors.size(); n++) {
                        current_score += old_scores->score(neighbors[n].x, neighbors[n].y, neighbors[n].z);
                    }
                    current_score *= multiplier;
                    current_score += old_scores->score(i,j,k);
                    scores->score(i,j,k) = current_score;
                }
            }
        }
    }
    return scores;
}

/**
    Finds the direction of the exposed face of the key piece that isn't bad_normal.

    @param voxel_list A VoxelGrid representing the current state of the puzzle.
    @param voxel The seed voxel.
    @param bad_normal The direction which the key is going to be removed from.
    @return The direction of the exposed face of the piece that isn't bad_normal.
*/
Voxel findNormal( CompFab::VoxelGrid * voxel_list, Voxel voxel, Voxel bad_normal) {
    int nx = voxel_list->m_dimX;
    int ny = voxel_list->m_dimY;
    int nz = voxel_list->m_dimZ;
    
    if ( bad_normal.x != -1 ) {
        if (voxel.x == 0 || voxel_list->isInside(voxel.x-1,voxel.y,voxel.z) != 1) {
            return Voxel(-1, 0, 0);
        }
    }
    if ( bad_normal.x != 1 ) {
        if (voxel.x == nx-1 || voxel_list->isInside(voxel.x+1,voxel.y,voxel.z) != 1) {
            return Voxel(1, 0, 0);
        }
    }
    if ( bad_normal.y != -1) {
        if (voxel.y == 0 || voxel_list->isInside(voxel.x,voxel.y-1,voxel.z) != 1) {
            return Voxel(0, -1, 0);
        }
    }
    if ( bad_normal.y != 1) {
        if (voxel.y == ny-1 || voxel_list->isInside(voxel.x,voxel.y+1,voxel.z) != 1) {
            return Voxel(0, 1, 0);
        }
    }
    if ( bad_normal.z != -1) {
        if (voxel.z == 0 || voxel_list->isInside(voxel.x,voxel.y,voxel.z-1) != 1) {
            return Voxel(0, 0, -1); 
        }
    }
    if ( bad_normal.z != 1) {
        if (voxel.z == nz-1 || voxel_list->isInside(voxel.x,voxel.y,voxel.z+1) != 1) {
            return Voxel(0, 0, 1);
        }
    }
    std::cout << "error" << std::endl;
    return Voxel(0,0,0);
}

/**
    A sorting function for std::sort for the VoxelPair class.

    @param i One of the VoxelPair being compared.
    @param j One of the VoxelPair being compared.
    @return true if i's blockee has a lower accessibility score than j's, false otherwise.
*/
bool accessSort(VoxelPair i, VoxelPair j) { return (i.blockee_accessibility < j.blockee_accessibility); }

/**
    Performs breadth first search to find potential voxels to block removal in the specified direction.

    @param voxel_list A VoxelGrid representing the current state of the puzzle.
    @param scores An AccessibilityStruct representing the current accesibility scores of the puzzle.
    @param seed The seed voxel for the key piece.
    @param bad_normal The direction which is being blocked.
    @param nb_one The limit of VoxelPairs to find to block removal.
    @param nb_two How many VoxelPairs to return after a sorting.
    @return The top nb_two VoxelPairs to block removal in direction bad_direction.
*/
std::vector<VoxelPair> bfs(CompFab::VoxelGrid * voxel_list, AccessibilityGrid * scores, Voxel seed, Voxel bad_normal, int nb_one, int nb_two) {
    std::vector<VoxelPair> potentials;
    int nx = voxel_list->m_dimX;
    int ny = voxel_list->m_dimY;
    int nz = voxel_list->m_dimZ;
    int size = nx*ny*nz;
    
    Voxel normal = findNormal(voxel_list, seed, bad_normal);

    // Mark all the vertices as not visited
    bool *visited = new bool[size];
    for(unsigned int i=0; i<size; ++i) {
        visited[i] = false;
    }
    
    // Create a queue for BFS
    std::list<Voxel> queue;
    std::vector<Voxel> neighbors;
    Voxel blockee;
    Voxel blocker;
    //Mark the current node as visited and enqueue it
    visited[seed.z*(nx*ny) + seed.y*ny + seed.x] = true;
    queue.push_back(seed);
    
    int count = 0;
    while (!queue.empty() && potentials.size() < nb_one) {
        count++;
        blockee = queue.front();
        blocker = blockee + normal;
        if (blocker.x < 0 || blocker.x >= nx || blocker.y < 0 || blocker.y >= ny || blocker.z < 0 || blocker.z >= nz) {
            ;
        } else if (voxel_list->isInside(blocker.x, blocker.y, blocker.z) == 1 && blocker != seed) {
            potentials.push_back(VoxelPair(blocker, scores->score(blocker.x, blocker.y, blocker.z), blockee, scores->score(blockee.x, blockee.y, blockee.z)));
        }
        queue.pop_front();
        neighbors = getNeighbors(blockee, voxel_list, 1);
        for (int i = 0; i < neighbors.size(); i++) {
            if ( !visited[ neighbors[i].z*(nx*ny) + neighbors[i].y*ny + neighbors[i].x ] ) {
                visited[ neighbors[i].z*(nx*ny) + neighbors[i].y*ny + neighbors[i].x ] = true;
                queue.push_back(neighbors[i]);
            }
        }
    }
    if (debug) {
        std::cout << "in bfs, went through: " << count << std::endl;
    }

    std::sort (potentials.begin(), potentials.end(), accessSort);
    std::vector<VoxelPair> accessible;
    for (int i = 0; i< potentials.size() && i < nb_two; i++) {
        accessible.push_back(potentials[i]);
    }
    delete[] visited;
    return accessible;
}

/**
    Uses breadth first search to find the shortest path from the seed to the blockee voxel specficied.

    @param voxel_list A VoxelGrid representing the current state of the puzzle.
    @param seed The seed voxel from which the search starts.
    @param goal The VoxelPair who's blockee voxel we're searching for.
    @param anchors A list of forbidden voxels to ensure interlocking.
    @param direction The direction which the piece is being removed.
    @return The shortest path from seed to goal.blockee including all other pieces that must be included to ensure removability.
*/
std::vector<Voxel> shortestPath(CompFab::VoxelGrid * voxel_list, Voxel seed, VoxelPair goal, std::vector<Voxel> anchors, Voxel direction) {
    if (debug) {
        std::cout << "in shortestPath" << std::endl;
        std::cout << "the seed is " << seed.toString() << std::endl;
        std::cout << "the goal is" << goal.toString() << std::endl;
        std::cout << "\tdirection of removal is " << direction.toString() << std::endl;
    }
    int nx = voxel_list->m_dimX;
    int ny = voxel_list->m_dimY;
    int nz = voxel_list->m_dimZ;
    int size = nx*ny*nz;
    
    std::vector<Voxel> path;
    // Mark all the vertices as not visited
    bool *visited = new bool[size];
    for(unsigned int i=0; i<size; ++i) {
        visited[i] = false;
    }
    // Create a queue for BFS
    std::list<std::vector<Voxel>> queue;
    std::vector<Voxel> neighbors;
    Voxel blockee = goal.blockee; // find the blockee
    Voxel blocker = goal.blocker; // don't go under or through blocker
    std::vector<Voxel> current;

    //Mark the current node as visited and enqueue it
    visited[seed.z*(nx*ny) + seed.y*ny + seed.x] = true;
    path.push_back(seed);
    queue.push_back(path);
    
    std::vector<Voxel> final_path;
    std::vector<Voxel> shortest_path;
    // Add blocker and all things below it to visited
    // generalize??

    Voxel end = blocker;
    Voxel neg_dir(direction.x*-1, direction.y*-1, direction.z*-1);
    while (end.x < nx && end.x > -1 && end.y > -1 && end.y < ny && end.z > -1 && end.z < nz) {
        if (debug) {
            std::cout << "\tsetting " << end.toString() << " to visited" <<std::endl;
        }
        visited[end.z*(nx*ny) + end.y*ny + end.x] = true;
        end += neg_dir;
    }
    // add anchors
    // ok so this is an issue, adding everything below anchors instead of general
    for (int i = 0; i < anchors.size(); i++) {
        end = anchors[i];
        while (end.x < nx && end.x > -1 && end.y > -1 && end.y < ny && end.z > -1 && end.z < nz) {
            visited[end.z*(nx*ny) + end.y*ny + end.x] = true;
            end += neg_dir;
        }
    }
    
    int count = 0;
    while ( !queue.empty() ) {
        count++;
        current = queue.front();
        if (current.back() == blockee) {
            shortest_path = current;
            break; 
        }
         
        queue.pop_front();
        neighbors = getNeighbors(current.back(), voxel_list, 1);
        if (debug) {
            std::cout << "neighbors of " << current.back().toString() << ": " << std::endl;
            printList(neighbors);
        }
        for (int i = 0; i < neighbors.size(); i++) {
            if ( !visited[ neighbors[i].z*(nx*ny) + neighbors[i].y*ny + neighbors[i].x ] ) {
                visited[ neighbors[i].z*(nx*ny) + neighbors[i].y*ny + neighbors[i].x ] = true;
                std::vector<Voxel> new_path = current;
                new_path.push_back(neighbors[i]);
                queue.push_back(new_path);
            }
        }
    }
    if (debug) {
        std::cout << "initial path is" << std::endl;
        printList(shortest_path);
    }
    // add things in the direction
    for (int i = 0; i < shortest_path.size(); i++) {
        end = Voxel(shortest_path[i].x, shortest_path[i].y, shortest_path[i].z);
        final_path.push_back(end);
        end += direction;
        while (end.x < nx && end.x > -1 && end.y > -1 && end.y < ny && end.z > -1 && end.z < nz) {
            if ( voxel_list->isInside(end.x, end.y, end.z) == 1 /*&& !visited[end.z*(nx*ny) + ny*end.y + end.x] */) {
                final_path.push_back(end);
            }
            end += direction;
        }
    }
    if (debug) {
        std::cout << "final path:" << std::endl;
        printList(final_path);
    }
    // Now... make sure that piece is connected after adding things. Otherwise need more path finding.
    //final_path = ensurePieceConnectivity(voxel_list, final_path, direction);
    delete[] visited;
    return final_path;
}

/**
    Finds the anchor voxels for the key piece.

    @param voxel_list A VoxelGrid representing the current state of the puzzle.
    @param seed The chosen seed voxel.
    @param normal_one The direction from which the piece is exposed, though cannot be removed.
    @param normal_two The direction from which the piece is being removed.
    @return A list of forbidden anchor voxels which search algorithms should not be allowed to access.
*/
std::vector<Voxel> findAnchors(CompFab::VoxelGrid * voxel_list, Voxel seed, Voxel normal_one, Voxel normal_two) {
    std::vector<Voxel> anchors;
    int nx = voxel_list->m_dimX;
    int ny = voxel_list->m_dimY;
    int nz = voxel_list->m_dimZ;

    if ( normal_one.x != -1 && normal_two.x != -1) {
        for (int i = 0; i < seed.x; i++) {
            if (voxel_list->isInside(i, seed.y, seed.z) == 1) {
                //check connectivity
                anchors.push_back(Voxel(i, seed.y, seed.z));
                break;
            }
        }
    }
    if ( normal_one.x != 1 && normal_two.x != 1) {
        for (int i = nx-1; i > seed.x; i--) {
            if (voxel_list->isInside(i, seed.y, seed.z) == 1) {
                //check connectivity
                anchors.push_back(Voxel(i, seed.y, seed.z));
                break;
            }
        }
    }
    if ( normal_one.y != -1 && normal_two.y != -1) {
        for (int i = 0; i < seed.y; i++) {
            if (voxel_list->isInside(seed.x, i, seed.z) == 1) {
                //check connectivity
                anchors.push_back(Voxel(seed.x, i, seed.z));
                break;
            }
        }
    }
    if ( normal_one.y != 1 && normal_two.y != 1) {
        for (int i = ny-1; i > seed.y; i--) {
            if (voxel_list->isInside(seed.x, i, seed.z) == 1) {
                //check connectivity
                anchors.push_back(Voxel(seed.x, i, seed.z));
                break;
            }
        }
    }
    if ( normal_one.z != -1 && normal_two.z != -1) {
        for (int i = 0; i < seed.z; i++) {
            if (voxel_list->isInside(seed.x, seed.y, i) == 1) {
                //check connectivity
                anchors.push_back(Voxel(seed.x, seed.y, i));
                break;
            }
        }
    }
    if ( normal_one.z != 1 && normal_two.z != 1) {
        for (int i = nz-1; i > seed.z; i--) {
            if (voxel_list->isInside(seed.x, seed.y, i) == 1) {
                //check connectivity
                anchors.push_back(Voxel(seed.x, seed.y, i));
                break;
            }
        }
    }
    return anchors;
}

/**
    Takes possible routes to block the key in direction normal_one and chooses the best.

    @param voxel_list A VoxelGrid representing the current state of the puzzle.
    @param scores An AccessibilityStruct representing the current accesibility scores of the puzzle.
    @param seed The chosen seed of the key.
    @param candidates Potential VoxelPairs to block the key.
    @param normal_one The direction from which to block the key.
    @param normal_two The direction from which the piece will be removed.
    @param index An integer pointer which gets set to the index of the chosen candidate.
    @return The path from the seed to the chosen VoxelPair blockee, which is now the key.
*/
std::vector<Voxel> filterKey(CompFab::VoxelGrid * voxel_list, AccessibilityGrid * scores, Voxel seed, 
                            std::vector<VoxelPair> candidates, Voxel normal_one, Voxel normal_two, int * index) {
    if (debug) {
        std::cout << "in filterKey" << std::endl;
    }
    std::vector<Voxel> chosen_path;
    std::vector<Voxel> path;
    std::vector<Voxel> anchors = findAnchors(voxel_list, seed, normal_one, normal_two);
    double sum;
    double max_score = std::numeric_limits<double>::max();
    for (int i = 0; i<candidates.size(); i++) {
        if (debug) {
            std::cout << "finding path to " << candidates[i].blockee.toString() << ", blocker is " << candidates[i].blocker.toString() << std::endl;
        }
        path = shortestPath(voxel_list, seed, candidates[i], anchors, Voxel(0, 0, 1));
        sum = 0;
        for (int j = 0; j < path.size(); j++) {
            sum += scores->score(path[j].x, path[j].y, path[j].z);
        }
        if (sum < max_score && sum > 0) {
            chosen_path = path;
            *index = i;
        }
    }
    return chosen_path;
}

/**
    Finds the final anchor voxel to block in the exposed direction

    @param voxel_list A VoxelGrid representing the current state of the puzzle.
    @param seed The chosen seed for the key.
    @param normal The direction which the VoxelPair blocks.
    @return The final anchor piece for the key.

*/
Voxel finalAnchor(CompFab::VoxelGrid * voxel_list, Voxel seed, VoxelPair blocks, Voxel normal) {
    int nx = voxel_list->m_dimX;
    int ny = voxel_list->m_dimY;
    int nz = voxel_list->m_dimZ;
    
    Voxel choice(-1, -1, -1);

    while (!(seed.x <= 0 || seed.x >= nx-1 || seed.y <= 0 || seed.y >= ny-1 || seed.z <= 0 || seed.z >= nz-1)) {
        seed += normal;
        if (voxel_list->isInside(seed.x, seed.y, seed.z) == 1) {
            choice = seed;
        }
    }

    if (choice == Voxel(-1, -1, -1)) {
        return blocks.blocker;
    } else {
        return choice;
    }
}

/**
    Expands a piece to have at least num_voxel voxels in it.

    @param voxel_list A VoxelGrid representing the current state of the puzzle.
    @param scores An AccessibilityStruct representing the current accesibility scores of the puzzle.
    @param key The piece being expanded.
    @param anchors The anchor voxels which should not be added to the piece.
    @param num_voxels The total number of voxels which should be in the piece.
    @param normal The direction the piece is being removed.
    @return An updated list of voxels that belong to this piece.
*/
std::vector<Voxel> expandPiece( CompFab::VoxelGrid * voxel_list, AccessibilityGrid * scores, std::vector<Voxel> key, std::vector<Voxel> anchors, int num_voxels, Voxel normal) {
    if (debug) {
        std::cout << "in expandPiece" << std::endl;
    }
    int nx = voxel_list->m_dimX;
    int ny = voxel_list->m_dimY;
    int nz = voxel_list->m_dimZ;
    int grid_size = nx*ny*nz;

    std::vector<Voxel> path;
    // Mark all the vertices as not visited
    bool *visited = new bool[grid_size];
    for(unsigned int i=0; i<grid_size; ++i) {
        visited[i] = false;
    }
    // ERROR Z DETECTED
    Voxel end;
    Voxel neg_dir = Voxel(normal.x*-1, normal.y*-1, normal.z*-1);
    for (int i = 0; i < anchors.size(); i++) {
        end = anchors[i];
        while (end.x < nx && end.x > -1 && end.y > -1 && end.y < ny && end.z > -1 && end.z < nz) {
            visited[end.z*(nx*ny) + end.y*ny + end.x] = true;
            end += neg_dir;
        }
    }
    for (int i = 0; i < key.size(); i++) {
        visited[key[i].z*nx*ny + key[i].y*ny + key[i].x] = true;
    }

    int count = key.size();
    int initial_count = count;
    std::vector<Voxel> neighbors;
    std::vector<Voxel> candidates;
    double sum, dist, random, accum;
    std::vector<double> probabilities;
    int total;
    std::vector<double> access_sums;
    double B = -2.0;
    int choice = -1;
    std::vector<Voxel> tempPiece;
    while (count < num_voxels) {
        for (int i = 0; i< key.size(); i++) {
            neighbors = getNeighbors(key[i], voxel_list, 1);
            for (int j = 0; j < neighbors.size(); j++) {
                if ( !visited[neighbors[j].z*nx*ny + neighbors[j].y*ny + neighbors[j].x] ) {
                    visited[neighbors[j].z*nx*ny + neighbors[j].y*ny + neighbors[j].x] = true;
                    candidates.push_back(neighbors[j]);
                }
            }
            neighbors.clear();
        }
        if (candidates.size() == 0) {
            break;
        }
        // Get score of candidate additions
        for (int i = 0; i < candidates.size(); i++) {
            tempPiece.clear();
            visited[candidates[i].z*nx*ny + candidates[i].y*ny + candidates[i].x] = false;
            sum = 0;
            total = count;
            
            // generalize to all
            end = candidates[i];
            while (end.x < nx && end.x > -1 && end.y > -1 && end.y < ny && end.z > -1 && end.z < nz) {
               if ((voxel_list->isInside(end.x, end.y, end.z) == 1) && (!visited[end.z*nx*ny + end.y*ny + end.x] )) {
                   tempPiece.push_back(end);
                   total++;
                   //sum += scores->score(end.x, end.y, end.z);
                } 
                end += normal;
            }
            tempPiece = ensurePieceConnectivity(voxel_list, tempPiece, normal);
            for (int j = 0; j < tempPiece.size(); j++) {
                sum += scores->score(tempPiece[j].x, tempPiece[j].y, tempPiece[j].z);
            }

            if (total > num_voxels) {
                continue;
            }
            
            access_sums.push_back(sum);
        }
        // Normalize scores
        dist = 0;
        for (int i = 0; i < access_sums.size(); i++) {
            access_sums[i] = std::pow(access_sums[i], B);
            dist += access_sums[i];
        }
        // Find probabilities
        for (int i = 0; i < access_sums.size(); i++) {
            probabilities.push_back( access_sums[i] / dist );
        }
       
        // Choose
        random = (double) std::rand() / (RAND_MAX);
        accum = 0.0;
        for (int i = 0; i < probabilities.size(); i++) {
            if ( random < accum + probabilities[i] || probabilities.size() == 1) {
                choice = i;
                break;
            } else {
                accum += probabilities[i];
            }
        }
        // Add choice to the key
        end = candidates[choice];
        while (end.x < nx && end.x > -1 && end.y > -1 && end.y < ny && end.z > -1 && end.z < nz) {
            if ((voxel_list->isInside(end.x, end.y, end.z) == 1) && (!visited[end.z*nx*ny + end.y*ny + end.x] )) {
                key.push_back(end);
                visited[end.z*nx*ny + end.y*ny + end.x] = true; 
            }
            end += normal;
        }
        
        for (int i = 0; i < key.size(); i++) {
            visited[key[i].z*nx*ny + key[i].y*ny + key[i].x] = true;
        }

        candidates.clear();
        access_sums.clear();
        probabilities.clear();
        count = key.size();
    }
    if (debug) {
        std::cout << "added " << std::to_string(key.size() - initial_count) << " voxels. Piece size is now " << std::to_string(key.size()) <<  std::endl;
        printList(key);
    }
    return key;
}

/**
    Verifies that the remainder is simply connected, i.e. all voxels can be reached from a randomly chosen voxel in the remainder.

    @param voxel_list A VoxelGrid representing the current state of the puzzle.
    @param piece The piece being verified.
    @return true if the piece is connected, false otherwise.
*/
bool verifyPiece( CompFab::VoxelGrid * voxel_list, std::vector<Voxel> piece) {
    if (debug) {
        std::cout << "in verifyPiece" << std::endl;
    }
    int nx = voxel_list->m_dimX;
    int ny = voxel_list->m_dimY;
    int nz = voxel_list->m_dimZ;
    int grid_size = nx*ny*nz;
        

    bool *visited = new bool[grid_size];
    bool *inPiece = new bool[grid_size];
    for(unsigned int i=0; i<grid_size; ++i) {
        inPiece[i] = false;
        visited[i] = false;
    }
    
    for (int i = 0; i < piece.size(); i++) {
        inPiece[i] = true;
        visited[piece[i].z*nx*ny + piece[i].y*ny + piece[i].x] = true;
    }
    
    // Find voxel to start bfs from
    int x = -1;
    int y = -1;
    int z = -1;
    for (int i = 0; i < nx; i++) {
        for (int j = 0; j < ny; j++) {
            for (int k = 0; k < nz; k++) {
                if (voxel_list->isInside(i, j, k) == 1 && !visited[k*ny*nx + j*ny + i]) {
                    x = i;
                    y = j;
                    z = k;
                    break;
                }
            }
            if (x != -1) {
                break;
            }
        }
        if (x != -1) {
            break;
        }
    }

    // Now, run bfs
    Voxel current;
    std::list<Voxel> queue;
    std::vector<Voxel> neighbors;
    visited[z*(nx*ny) + y*ny + x] = true;
    queue.push_back(Voxel(x, y, z));
    while (!queue.empty()) {
        current = queue.front();
        queue.pop_front();
        neighbors = getNeighbors(current, voxel_list, 1);
        for (int i = 0; i < neighbors.size(); i++) {
            if ( !visited[ neighbors[i].z*(nx*ny) + neighbors[i].y*ny + neighbors[i].x ] ) {
                visited[ neighbors[i].z*(nx*ny) + neighbors[i].y*ny + neighbors[i].x ] = true;
                queue.push_back(neighbors[i]);
            }
        }
    }
    bool result = true;
    for (int i = 0; i < nx; i++) {
        for (int j = 0; j < ny; j++) {
            for (int k = 0; k < nz; k++) {
                if (voxel_list->isInside(i,j,k) == 1 && !visited[k*(ny*nx) + j*ny + i] ) {
                    if (debug) {
                        std::cout << "piece could not be verfied due to " << Voxel(i,j,k).toString() << std::endl;
                    }
                    result = false;
                }
             }
        }
    }
    return result;
}

/**
    Finds the normal direction from which to remove the next piece.

    @param voxel_list A VoxelGrid representing the current state of the puzzle.
    @param voxel The voxel who's normal direction we're finding
    @param piece The previous piece in the puzzle.
    @return The direction from which the piece will be removed.
*/
Voxel findNormalDirection( CompFab::VoxelGrid * voxel_list, Voxel voxel, std::vector<Voxel> piece) {
    if (debug) {
        std::cout << "in findNormalDirection" << std::endl;
    }
    int nx = voxel_list->m_dimX;
    int ny = voxel_list->m_dimY;
    int nz = voxel_list->m_dimZ;
    int size = nx*ny*nz;
    
    bool *visited = new bool[size];
    for(unsigned int i=0; i<size; ++i) {
        visited[i] = false;
    }
    
    // set voxels of piece to true
    for (int i = 0; i < piece.size(); i++) {
        visited[piece[i].z*ny*nx + piece[i].y*ny + piece[i].x] = true;
    }
    
    if (visited[voxel.z*ny*nx + voxel.y*ny + voxel.x-1]) {
        return Voxel(-1, 0, 0);
    } else if (visited[voxel.z*ny*nx + voxel.y*ny + voxel.x+1]) {
        return Voxel(1, 0, 0);
    } else if (visited[voxel.z*ny*nx + (voxel.y-1)*ny + voxel.x]) {
        return Voxel(0, -1, 0);
    } else if (visited[voxel.z*ny*nx + (voxel.y+1)*ny + voxel.x]) {
        return Voxel(0, 1, 0);
    } else if (visited[(voxel.z-1)*ny*nx + voxel.y*ny + voxel.x]) {
        return Voxel(0, 0, -1);
    } else if (visited[(voxel.z+1)*ny*nx + voxel.y*ny + voxel.x]) {
        return Voxel(0, 0, 1);
    } else {
        std::cout << "Error finding normal" << std::endl;
        return Voxel(-1,-1,-1);
    }
} 

/**
    A sorting function for std::sort for the VoxelSort class.

    @param i The VoxelSort being compared.
    @param j The other VoxelSort being compared.
    @return true if i's accessibility score is less than j's, false otherwise.
*/
bool voxelSortSorter(VoxelSort i, VoxelSort j) { return (i.score < j.score); }

/**
    Sorts the potential seeds for a piece by their accessibility scores.
    
    @param voxel_list A VoxelGrid representing the current state of the puzzle.
    @param scores An AccessibilityStruct representing the current accesibility scores of the puzzle.
    @param seeds A list of potential seeds for a piece.
    @param piece The previous piece.
    @return A list of potential seeds sorted by their accessibility scores.
*/
std::vector<Voxel> seedSorter(CompFab::VoxelGrid * voxel_list, AccessibilityGrid * scores, std::vector<Voxel> seeds, std::vector<Voxel> piece) {
    if (debug) {
        std::cout << "in seedSorter" << std::endl;
    }
    int nx = voxel_list->m_dimX;
    int ny = voxel_list->m_dimY;
    int nz = voxel_list->m_dimZ;
    std::vector<VoxelSort> voxels;

    int max_distance = 0;
    double max_dist_double = 0.0;
    double max_access = 0.0;
    int indiv_dist;
    // find max distance
    Voxel normal;
    Voxel end;
    for (int i = 0; i < seeds.size(); i++) {
        normal = findNormalDirection( voxel_list, seeds[i], piece);
        indiv_dist = 0;
        end = Voxel(seeds[i].x, seeds[i].y, seeds[i].z);
        for (int j = 0; end.x < nx && end.x > -1 && end.y > -1 && end.y < ny && end.z > -1 && end.z < nz ; j++) {
            if (voxel_list->isInside(end.x, end.y, end.z) == 1) {
                indiv_dist = j;
            }
            end += normal;
        }
        if ( max_distance < indiv_dist) {
            max_distance = indiv_dist;
        }
    }
    max_dist_double = (double)max_distance;
    // find max accessibilty score
    for (int i = 0; i < seeds.size(); i++) {
        if ( max_access < scores->score(seeds[i].x, seeds[i].y, seeds[i].z) ) {
            max_access = scores->score( seeds[i].x, seeds[i].y, seeds[i].z );
        }
    }
    // now, calculate score for each thing with normalized values
    double this_score;
    double this_dist;
    for (int i = 0; i < seeds.size(); i++) {
        this_score = scores->score( seeds[i].x, seeds[i].y, seeds[i].z ) / max_access;
        
        normal = findNormalDirection( voxel_list, seeds[i], piece);
        indiv_dist = 0;
        for (int j = 0; end.x < nx && end.x > -1 && end.y > -1 && end.y < ny && end.z > -1 && end.z < nz ; j++) {
            if (voxel_list->isInside(end.x, end.y, end.z) == 1) {
                indiv_dist = j;
            }
            end += normal;
        }
        this_dist = (double)indiv_dist / max_dist_double;
        voxels.push_back(VoxelSort(seeds[i], this_score+this_dist));
    }

    std::sort (voxels.begin(), voxels.end(), voxelSortSorter);
    std::vector<Voxel> finalVoxels;
    for (int i = 0; i < voxels.size(); i++) {
        finalVoxels.push_back(voxels[i].voxel);
    }
    return finalVoxels;
}

/**
    Finds potential seeds for the next piece.

    @param voxel_list A VoxelGrid representing the current state of the puzzle.
    @param scores An AccessibilityStruct representing the current accesibility scores of the puzzle.
    @param piece The previous piece.
    @param perpendicular The directions from which the next piece cannot be removed.
    @return A list of potential seeds for the next piece.
*/
std::vector<Voxel> findCandidateSeeds(CompFab::VoxelGrid * voxel_list, AccessibilityGrid * scores, std::vector<Voxel> piece, Voxel perpendicular) {
    if (debug) {
        std::cout << "in findCandidateSeeds" << std::endl;
    }
    std::vector<Voxel> neighbors;
    int nx = voxel_list->m_dimX;
    int ny = voxel_list->m_dimY;
    int nz = voxel_list->m_dimZ;
    int size = nx*ny*nz;
    Voxel voxel;
    // Mark all the vertices as not visited
    bool *visited = new bool[size];
    for(unsigned int i=0; i<size; ++i) {
        visited[i] = false;
    }
    for (int i = 0; i < piece.size(); i++) {
        visited[piece[i].z*ny*nx + piece[i].y*ny + piece[i].x] = true;
        // mark all pieces in normal direction as visited too
        voxel = piece[i] + Voxel(-1*perpendicular.x, -1*perpendicular.y, -1*perpendicular.z);
        if ( voxel.x > -1 && voxel.x < nx && voxel.y > -1 && voxel.y < ny && voxel.z > -1 && voxel.z < nz ) {
            visited[voxel.z*ny*nx + voxel.y*ny + voxel.x] = true;
        }
    }


    for (int i = 0; i < piece.size(); i++) {
        voxel = Voxel(piece[i].x, piece[i].y, piece[i].z);
        
        if ( voxel.x != 0) {
            if (voxel_list->isInside(voxel.x-1,voxel.y,voxel.z) == 1 && !visited[voxel.z*ny*nx + voxel.y*ny + voxel.x-1]) {
                visited[voxel.z*ny*nx + voxel.y*ny + voxel.x-1] = true;
                if (perpendicular.x == 0) {
                    neighbors.push_back(Voxel(voxel.x-1,voxel.y,voxel.z));
                }
            }
        }
        
        if ( voxel.x != nx-1) {
            if (voxel_list->isInside(voxel.x+1,voxel.y,voxel.z) == 1 && !visited[voxel.z*ny*nx + voxel.y*ny + voxel.x+1]) {
                visited[voxel.z*ny*nx + voxel.y*ny + voxel.x+1] = true;
                if (perpendicular.x == 0) {
                    neighbors.push_back(Voxel(voxel.x+1,voxel.y,voxel.z));
                }
            }
        }
        
        if (voxel.y != 0) {
            if (voxel_list->isInside(voxel.x,voxel.y-1,voxel.z) == 1 && !visited[voxel.z*ny*nx + (voxel.y-1)*ny + voxel.x]) {
                visited[voxel.z*ny*nx + (voxel.y-1)*ny + voxel.x] = true;
                if (perpendicular.y == 0) {
                    neighbors.push_back(Voxel(voxel.x,voxel.y-1,voxel.z));
                }
            }
        }
        
        if (voxel.y != ny-1) {
            if (voxel_list->isInside(voxel.x,voxel.y+1,voxel.z) == 1 && !visited[voxel.z*ny*nx + (voxel.y+1)*ny + voxel.x]) {
                visited[voxel.z*ny*nx + (voxel.y+1)*ny + voxel.x] = true;
                if (perpendicular.y == 0) {
                    neighbors.push_back(Voxel(voxel.x,voxel.y+1,voxel.z));
                }
            }
        }
        
        if (voxel.z != 0) {
            if (voxel_list->isInside(voxel.x,voxel.y,voxel.z-1) == 1 && !visited[(voxel.z-1)*ny*nx + voxel.y*ny + voxel.x]) {
                visited[(voxel.z-1)*ny*nx + voxel.y*ny + voxel.x] = true;
                if (perpendicular.z == 0) {
                    neighbors.push_back(Voxel(voxel.x,voxel.y,voxel.z-1));
                }
            }
        }
        if (voxel.z != nz-1) {
            if (voxel_list->isInside(voxel.x,voxel.y,voxel.z+1) == 1 && !visited[(voxel.z+1)*ny*nx + voxel.y*ny + voxel.x]) {
                visited[(voxel.z+1)*ny*nx + voxel.y*ny + voxel.x] = true;
                if (perpendicular.z == 0) {
                    neighbors.push_back(Voxel(voxel.x,voxel.y,voxel.z+1));
                }
            }
        }
    }
    if (neighbors.size() > 10) {
        std::vector<Voxel> sorted = seedSorter(voxel_list, scores, neighbors, piece);
        std::vector<Voxel> topX;
        for (int i = 0; i < 10 && i < sorted.size(); i++) {
            topX.push_back(sorted[i]);
        }
        
        if (debug) {
            std::cout << "candidate seeds are: " << std::endl;
            printList(topX);
        }
        return topX;
    } else {
        if (debug) {
            std::cout << "candidate seeds are: " << std::endl;
            printList(neighbors);
        }
        return neighbors;
    }
}

/**
    Finds teh shorest path from one voxel to another.
    @param voxel_list A VoxelGrid representing the current state of the puzzle.
    @param start The voxel the path starts from.
    @param goal The voxel to find.
    @return The path from start voxel to goal voxel.
*/
std::vector<Voxel> shortestPathTwo(CompFab::VoxelGrid * voxel_list, Voxel start, Voxel goal) {
    if (debug) {
        std::cout << "in shortestPathTwo" << std::endl;
        std::cout << "start is " << start.toString() << ", goal is " << goal.toString() << std::endl;
    }
    int nx = voxel_list->m_dimX;
    int ny = voxel_list->m_dimY;
    int nz = voxel_list->m_dimZ;
    int size = nx*ny*nz;

    bool *visited = new bool[size];
    for (int i = 0; i < size; i++) {
        visited[i] = false;
    }
    std::list<std::vector<Voxel>> queue;
    std::vector<Voxel> neighbors;
    std::vector<Voxel> current;
    std::vector<Voxel> path;

    //Mark the current node as visited and enqueue it
    visited[start.z*(nx*ny) + start.y*ny + start.x] = true;
    path.push_back(start);
    queue.push_back(path);

    std::vector<Voxel> shortest_path;

    while ( !queue.empty() ) {
        current = queue.front();
        if (current.back() == goal) {
            shortest_path = current;
            break;
        }

        queue.pop_front();
        neighbors = getNeighbors(current.back(), voxel_list, 1);
        for (int i = 0; i < neighbors.size(); i++) {
            if ( !visited[ neighbors[i].z*(nx*ny) + neighbors[i].y*ny + neighbors[i].x ] ) {
                visited[ neighbors[i].z*(nx*ny) + neighbors[i].y*ny + neighbors[i].x ] = true;
                std::vector<Voxel> new_path = current;
                new_path.push_back(neighbors[i]);
                queue.push_back(new_path);
            }
        }
    }
    if (queue.empty()) {
        std::cout << "\n\n\t\tFAILED FINDING PATH\n\t\tFAILED FINDING PATH\n\t\tFAILED FINDING PATH\n\n" << std::endl;
    }
    return shortest_path;
}

/**
    From a list of potential seed voxels, creates the initial design of a potential piece.

    @param voxel_list A VoxelGrid representing the current state of the puzzle.
    @param scores An AccessibilityStruct representing the current accesibility scores of the puzzle.
    @param prevPiece The previous piece.
    @param candidates A list of potential seeds for the next piece.
    @param index An integer pointer that gets set to the index of the chosen candidate.
    @return A list containing the initial construction of the next piece.
*/
std::vector<Voxel> createInitialPiece(CompFab::VoxelGrid * voxel_list, AccessibilityGrid * scores, std::vector<Voxel> prevPiece, std::vector<Voxel> candidates, int * index) {
    if (debug) {
        std::cout << "in createInitialPiece" << std::endl;
    }
    int nx = voxel_list->m_dimX;
    int ny = voxel_list->m_dimY;
    int nz = voxel_list->m_dimZ;
    int size = nx*ny*nz;
    
    std::vector<Voxel> bestChoice;
    std::vector<Voxel> currentChoice;
    std::vector<Voxel> toRemove;
    double bestScore = std::numeric_limits<double>::max();
    double currentScore;
    Voxel normal;
    Voxel end;
    bool *visited = new bool[size];
    std::vector<Voxel> shortestPath;
    std::vector<Voxel> finalPath;
    bool in;
    for (int i = 0; i < candidates.size(); i++) {
        //std::cout << "checking candidate " << candidates[i].toString() << std::endl;
        // Reset everything
        currentChoice.clear();
        toRemove.clear();
        shortestPath.clear();
        finalPath.clear();

        // Find the normal
        normal = findNormalDirection( voxel_list, candidates[i], prevPiece);
        end = candidates[i] + normal;
        //std::cout << "\tnormal is " << normal.toString() << std::endl;
        // Figure out what voxels must be added in the normal direction
        //std::cout << "\tfinding the voxels to add" << std::endl;
        for (int j = 0; end.x < nx && end.x > -1 && end.y > -1 && end.y < ny && end.z > -1 && end.z < nz ; j++) {
            //std::cout << "\tchecking " << end.toString() << std::endl;
            if (voxel_list->isInside(end.x, end.y, end.z) == 1) {
                //std::cout << "\t\t" << end.toString() << " must be removed" << std::endl;
                toRemove.push_back(end); 
            }
            end += normal;
        }
        
        if (toRemove.size() == 0) {
            currentScore = scores->score(candidates[i].x, candidates[i].y, candidates[i].z);
            currentChoice.push_back(candidates[i]);
            /*if (currentScore < bestScore) {
                bestScore = currentScore;
                currentChoice.push_back(candidates[i]);
                std::cout << "\t\t\tsetting bestChoice with nothing to remove with current score =" << std::to_string(currentScore)<< std::endl;
                bestChoice = currentChoice;
            }*/
        }
        
        //std::cout << "\tvoxels to remove: " << std::endl;
        //printList(toRemove);

        // Now, find shortest path to each of these and add said path to the piece
        for (int j = 0; j < toRemove.size(); j++) {
            shortestPath = shortestPathTwo(voxel_list, candidates[i], toRemove[j]);
            //std::cout << "\tpath to " << toRemove[j].toString() << " is: " << std::endl;
            if (shortestPath.size() == 0) {
                std::cout << "Something went wrong" << std::endl;
            }
            if (debug) {
                printList(shortestPath);
            }
            // add all voxels in normal direction
            for (int k = 0; k < shortestPath.size(); k++) {
                end = shortestPath[k];
                for (int l = 0; end.x < nx && end.x > -1 && end.y > -1 && end.y < ny && end.z > -1 && end.z < nz ; l++) {
                    if (voxel_list->isInside(end.x, end.y, end.z) == 1) {
                        finalPath.push_back(end);
                    }
                    end += normal;
                }
            }
            // add path to the piece
            for (int k = 0; k < finalPath.size(); k++) {
                // Check if it's already in there
                in = false;
                for (int l = 0; l < currentChoice.size(); l++) {
                    if (finalPath[k] == currentChoice[l]) {
                        in = true;
                        break;
                    }
                }
                if ( !in ) {
                    currentChoice.push_back(finalPath[k]);
                }
            }
        }

        // Now, decide if this one is better
        if (currentScore != 0.0) {
            currentScore = 0.0;
        }
        for (int j = 0; j < currentChoice.size(); j++) {
            currentScore += scores->score(currentChoice[j].x, currentChoice[j].y, currentChoice[j].z);
        }
        if (currentScore < bestScore && currentScore > 0.0) {
            //std::cout << "\t\t\tsetting bestChoice with current score =" << std::to_string(currentScore) <<  std::endl;
            bestScore = currentScore;
            bestChoice = currentChoice;
            *index = i;
        }
    }
    
    if (debug) {
        std::cout << "best choice is:" << std::endl;
        printList(bestChoice);
    }
    return bestChoice;
}

/**
    Finds a blocker for the next piece in direction toBlock

    @param voxel_list A VoxelGrid representing the current state of the puzzle.
    @param scores An AccessibilityStruct representing the current accesibility scores of the puzzle.
    @param seed A seed for the next piece.
    @param toBlock The direction that must be blocked.
    @param normal The direction the piece is being released in.
    @param nb_one The number of blocking pairs to find.
    @param nb_two The number of blocking pairs to choose amongst.
    @param anchor A Voxel pointer that gets set to the blocker to be an anchor later on.
    @param anchorList A list of anchors which any path finding is not allowed to go through.
    @return An update of the piece blocked in the direction toBlock.
*/
std::vector<Voxel>  bfsTwo(CompFab::VoxelGrid * voxel_list, AccessibilityGrid * scores, Voxel seed, Voxel toBlock, Voxel normal, int nb_one, int nb_two, Voxel * anchor, std::vector<Voxel> anchorList){
    if (debug) {
        std::cout << "in bfsTwo" << std::endl;
    }
    std::vector<VoxelPair> potentials;
    int nx = voxel_list->m_dimX;
    int ny = voxel_list->m_dimY;
    int nz = voxel_list->m_dimZ;
    int size = nx*ny*nz;


    // Mark all the vertices as not visited
    bool *visited = new bool[size];
    for(unsigned int i=0; i<size; ++i) {
        visited[i] = false;
    }

    // Create a queue for BFS
    std::list<Voxel> queue;
    std::vector<Voxel> neighbors;
    Voxel blockee;
    Voxel blocker;
    //Mark the current node as visited and enqueue it
    visited[seed.z*(nx*ny) + seed.y*ny + seed.x] = true;
    queue.push_back(seed);

    int count = 0;
    while (!queue.empty() && potentials.size() < nb_one) {
        count++;
        blockee = queue.front();
        blocker = blockee + toBlock;
        if (blocker.x < 0 || blocker.x >= nx || blocker.y < 0 || blocker.y >= ny || blocker.z < 0 || blocker.z >= nz) {
            ;
        } else if (voxel_list->isInside(blocker.x, blocker.y, blocker.z) == 1 && blocker != seed) {
            potentials.push_back(VoxelPair(blocker, scores->score(blocker.x, blocker.y, blocker.z), blockee, scores->score(blockee.x, blockee.y, blockee.z)));
        }
        queue.pop_front();
        neighbors = getNeighbors(blockee, voxel_list, 1);
        for (int i = 0; i < neighbors.size(); i++) {
            if ( !visited[ neighbors[i].z*(nx*ny) + neighbors[i].y*ny + neighbors[i].x ] ) {
                visited[ neighbors[i].z*(nx*ny) + neighbors[i].y*ny + neighbors[i].x ] = true;
                queue.push_back(neighbors[i]);
            }
        }
    }
    if (debug) {
        std::cout << "in bfsTwo, went through: " << count << std::endl;
    }

    std::sort (potentials.begin(), potentials.end(), accessSort);
    std::vector<VoxelPair> accessible;
    for (int i = 0; i< potentials.size() && nb_two < accessible.size(); i++) {
        if (debug) {
            std::cout << "\t" << potentials[i].toString() << " could block" << std::endl;
        }
        accessible.push_back(potentials[i]);
    }
    std::vector<Voxel> currentPiece;
    std::vector<Voxel> bestPiece;
    double currentScore;
    double bestScore = std::numeric_limits<double>::max();
    Voxel bestBlocker;

    Voxel current;
    bool skip;
    for (int i = 0; i < accessible.size(); i++) {
        currentPiece = shortestPath(voxel_list, seed, accessible[i], anchorList, normal);
        if (debug) {
            std::cout<< "path from " << seed.toString() << " to " << accessible[i].blockee.toString() << std::endl;
        }

        // Verify that the blocker isn't isolated, i.e. that it can access rest of puzzle not through the piece
        for(unsigned int j=0; j<size; ++j) {
            visited[j] = false;
        }
        for (int j = 0; j < currentPiece.size(); j++) {
            visited[ currentPiece[j].z*(ny*nx) + currentPiece[j].y*ny + currentPiece[j].x ] = true;
        }
        queue.clear();
        queue.push_back(accessible[i].blocker);
        while (!queue.empty()) {
            current = queue.front();
            queue.pop_front();
            neighbors = getNeighbors(current, voxel_list, 1);
            for (int k = 0; k < neighbors.size(); k++) {
                if ( !visited[ neighbors[k].z*(nx*ny) + neighbors[k].y*ny + neighbors[k].x ] ) {
                    visited[ neighbors[k].z*(nx*ny) + neighbors[k].y*ny + neighbors[k].x ] = true;
                    queue.push_back(neighbors[k]);
                }
            }
        }

        // Finally, make sure everything in puzzle was visited
        skip = false;
        for (int x = 0; x < nx; x++) {
            for (int y = 0; y < ny; y++) {
                for (int z = 0; z < nz; z++) {
                    if ((voxel_list->isInside(x, y, z) == 1) && !visited[z*(ny*nx) + y*ny + x]) {
                        skip = true;
                        if (debug) {
                            std::cout << "bad blocker is " << accessible[i].blocker.toString() << std::endl;
                        }
                     }
                }
            }
        }
        
        if (skip) continue;


        //printList(currentPiece);
        if (currentPiece.size() == 0) {
            std::cout << "Could not find path?" << std::endl;
            continue;
        }
        currentScore = 0.0;
        for (int j = 0; j < currentPiece.size(); j++) {
            currentScore += scores->score(currentPiece[j].x, currentPiece[j].y, currentPiece[j].z);
        }
        //std::cout << "\tthe score is " << std::to_string(currentScore) << std::endl;
        if (currentScore < bestScore) {
            bestScore = currentScore;
            bestPiece = currentPiece;
            bestBlocker = accessible[i].blocker;
        }
    }
    if (debug) {
        std::cout << "returning: " << std::endl;
        printList(bestPiece);
    }
    *anchor = bestBlocker;
    return bestPiece;
}

/**
    Ensures that the piece isn't mobile in a direction.

    @param voxel_list A VoxelGrid representing the current state of the puzzle.
    @param scores An AccessibilityStruct representing the current accesibility scores of the puzzle.
    @param prevPiece The previous piece.
    @param currentPiece The piece currently being built.
    @param prevPieceId The value which voxel_list->(x,y,z) is set to for the previous piece.
    @param dir The direction for which we're checking mobility.
    @param normal The direction the current piece is being removed in.
    @param prevNormal The direction the previous piece was removed in.
    @param anchor A Voxel pointer which gets set to the anchor for this direction.
    @param anchorList A list of anchors which the piece cannot travel through.
    @return An updated version of the current piece, only changed if it was not blocked in direction dir.
*/
std::vector<Voxel> mobilityCheck(CompFab::VoxelGrid * voxel_list, AccessibilityGrid * scores, std::vector<Voxel> prevPiece, std::vector<Voxel> currentPiece, int prevPieceId, Voxel dir, Voxel normal, Voxel prevNormal, Voxel * anchor, std::vector<Voxel> anchorList) {
    if (debug) {
        std::cout << "checking mobility in dir " << dir.toString() << std::endl;
    }
    int count = currentPiece.size();
    
    int nx = voxel_list->m_dimX;
    int ny = voxel_list->m_dimY;
    int nz = voxel_list->m_dimZ;
    int size = nx*ny*nz;

    bool *isCurrent = new bool[size];
    for(unsigned int i=0; i<size; ++i) {
        isCurrent[i] = false;
    }
    
    for (int i = 0; i < currentPiece.size(); i++) {
        isCurrent[currentPiece[i].z*(ny*nx) + currentPiece[i].y*ny + currentPiece[i].x] = true;
    }

    bool blocked = false;
    bool in;
    Voxel end;
    std::vector<Voxel> path;
    for (int i = 0; i < currentPiece.size(); i++) {
        end = currentPiece[i];
        while (end.x < nx && end.x > -1 && end.y > -1 && end.y < ny && end.z > -1 && end.z < nz && !blocked) { 
            if (voxel_list->isInside(end.x, end.y, end.z) == 1 || ((voxel_list->isInside(end.x, end.y, end.z) == prevPieceId) && (dir != prevNormal)) ) {
                if (!isCurrent[end.z*(ny*nx) + end.y*ny + end.x]) {
                    //*anchor = end;
                    blocked = true;
                }
            }
            end += dir;
        }
        if (blocked) {
            break;
        }
    }


    if (!blocked) {
        // add blockee voxel
        //std::cout << "NEED TO BLOCK" << std::endl;
        path = bfsTwo(voxel_list, scores, currentPiece[0], dir, normal, 50, 10, anchor, anchorList);
        //std::cout << "BEST CHOICE IS " << std::endl;
        //printList(path);
        for (int i = 0; i < path.size(); i++) {
            in = false;
            //std::cout << "\tchecking if " << path[i].toString() << "is already in piece" << std::endl;
            for (int j = 0; j < currentPiece.size(); j++) {
                if (path[i] == currentPiece[j]) {
                    //std::cout << "\t\t" << path[i].toString() << " is in current piece already" << std::endl;
                    in = true;
                    break;
                }
            }
            if ( !in ) {
                currentPiece.push_back(path[i]);
            }
        }
    }
    if (currentPiece.size() == count) {
        if (debug) { 
            std::cout << "no change" << std::endl;
        }
    } else {
        if (debug) {
            std::cout << "twas change" << std::endl;
        }
    }
    return currentPiece;
}

/**
    Ensures the piece is only mobile in one direction.

    @param voxel_list A VoxelGrid representing the current state of the puzzle.
    @param scores An AccessibilityStruct representing the current accesibility scores of the puzzle.
    @param prevPiece The previous piece.
    @param currentPiece The piece which is being checked.
    @param prevPieceId The value which voxel_list->(x,y,z) is set to for the previous piece.
    @param prevNormal The direction the previous piece was removed in.
    @param theAnchors A pointer to a voxel vector that gets updated with the voxels that cannot be added to this piece.
    @return An udpated version of the piece that is only mobile in one direction.
*/
std::vector<Voxel> ensureInterlocking(CompFab::VoxelGrid * voxel_list, AccessibilityGrid * scores, std::vector<Voxel> prevPiece, std::vector<Voxel> currentPiece, int prevPieceId, Voxel prevNormal, std::vector<Voxel> * theAnchors) {
    if (debug) {
        std::cout << "in ensureInterlocking" << std::endl;
    }
    Voxel normal = findNormalDirection( voxel_list, currentPiece[0], prevPiece);
    Voxel dir;
    if (debug) {
        std::cout << "normal for this whole piece starting at" << currentPiece[0].toString() << " is " << normal.toString() << std::endl;
    }
    // Perform mobility check in each of the other 5 directions
    Voxel anchor;
    std::vector<Voxel> anchorList;
    if (normal.x != -1) {
        dir = Voxel(-1, 0, 0);
        anchor = Voxel(-1, -1, -1);
        currentPiece = mobilityCheck(voxel_list, scores, prevPiece, currentPiece, prevPieceId, dir, normal, prevNormal, &anchor, anchorList);
        if (anchor != Voxel(-1, -1, -1)) {
            anchorList.push_back(anchor);
        }
    }
    if (normal.x != 1) {
        dir = Voxel(1, 0, 0);
        anchor = Voxel(-1, -1, -1);
        currentPiece = mobilityCheck(voxel_list, scores, prevPiece, currentPiece, prevPieceId, dir, normal, prevNormal, &anchor, anchorList);
        if (anchor != Voxel(-1, -1, -1)) {
            anchorList.push_back(anchor);
        }
    }
    if (normal.y != -1) {
        dir = Voxel(0, -1, 0);
        anchor = Voxel(-1, -1, -1);
        currentPiece = mobilityCheck(voxel_list, scores, prevPiece, currentPiece, prevPieceId, dir, normal, prevNormal, &anchor, anchorList);
        if (anchor != Voxel(-1, -1, -1)) {
            anchorList.push_back(anchor);
        }
    }
    if (normal.y != 1) {
        dir = Voxel(0, 1, 0);
        anchor = Voxel(-1, -1, -1);
        currentPiece = mobilityCheck(voxel_list, scores, prevPiece, currentPiece, prevPieceId, dir, normal, prevNormal, &anchor, anchorList);
        if (anchor != Voxel(-1, -1, -1)) {
            anchorList.push_back(anchor);
        }
    }
    if (normal.z != -1) {
        dir = Voxel(0, 0, -1);
        currentPiece = mobilityCheck(voxel_list, scores, prevPiece, currentPiece, prevPieceId, dir, normal, prevNormal, &anchor, anchorList);
        if (anchor != Voxel(-1, -1, -1)) {
            anchorList.push_back(anchor);
        }
    }
    if (normal.z != 1) {
        dir = Voxel(0, 0, 1);
        currentPiece = mobilityCheck(voxel_list, scores, prevPiece, currentPiece, prevPieceId, dir, normal, prevNormal, &anchor, anchorList);
        if (anchor != Voxel(-1, -1, -1)) {
            anchorList.push_back(anchor);
        }
    }
    *theAnchors = anchorList;
    return currentPiece;
}

/**
    Finds the shortest path from a voxel to another piece.

    @param voxel_list A VoxelGrid representing the current state of the puzzle.
    @param start The voxel the path starts on.
    @param goals The piece we're finding a path to.
    @return The path from voxel start to a piece.
*/
std::vector<Voxel> shortestPathThree(CompFab::VoxelGrid * voxel_list, Voxel start, std::vector<Voxel> goals) {
    if (debug) {
        std::cout << "in shortestPathThree" << std::endl;
        std::cout << "starting from " << start.toString() << std::endl;
    }
    int nx = voxel_list->m_dimX;
    int ny = voxel_list->m_dimY;
    int nz = voxel_list->m_dimZ;
    int size = nx*ny*nz;

    bool *visited = new bool[size];
    for (int i = 0; i < size; i++) {
        visited[i] = false;
    }
    
    bool *piece = new bool[size];
    for (int i =0; i < size; i++) {
        piece[i] = false;
    }

    for (int i = 0; i < goals.size(); i++) {
        piece[goals[i].z*(ny*nx) + goals[i].y*ny + goals[i].x] = true;
    }
    piece[start.z*(nx*ny) + start.y*ny + start.x] = false;

    std::list<std::vector<Voxel>> queue;
    std::vector<Voxel> neighbors;
    std::vector<Voxel> current;
    std::vector<Voxel> path;

    //Mark the current node as visited and enqueue it
    visited[start.z*(nx*ny) + start.y*ny + start.x] = true;
    path.push_back(start);
    queue.push_back(path);

    std::vector<Voxel> shortest_path;
    Voxel back;
    while ( !queue.empty() ) {
        current = queue.front();
        back = current.back();
        if ( piece[back.z*(ny*nx) + back.y*ny + back.x] ) {
            shortest_path = current;
            break;
        }

        queue.pop_front();
        neighbors = getNeighbors(current.back(), voxel_list, 1);
        for (int i = 0; i < neighbors.size(); i++) {
            if ( !visited[ neighbors[i].z*(nx*ny) + neighbors[i].y*ny + neighbors[i].x ] ) {
                visited[ neighbors[i].z*(nx*ny) + neighbors[i].y*ny + neighbors[i].x ] = true;
                std::vector<Voxel> new_path = current;
                new_path.push_back(neighbors[i]);
                queue.push_back(new_path);
            }
        }
    }
    if (queue.empty()) {
        std::cout << "\n\n\t\tFAILED FINDING PATH (3)\n\t\tFAILED FINDING PATH (3)\n\t\tFAILED FINDING PATH (3)\n\n" << std::endl;
    }
    return shortest_path;
}

/**
    Ensures that a piece is connected to itself.

    @param voxel_list A VoxelGrid representing the current state of the puzzle.
    @param piece The piece being checking.
    @param normal The direction the piece is being removed in.
    @return Updates the piece if it's not connected, otherwise leave it alone.
*/
std::vector<Voxel> ensurePieceConnectivity(CompFab::VoxelGrid * voxel_list, std::vector<Voxel> piece, Voxel normal) {
    if (debug) {
        std::cout << "in ensurePieceConnectivity" << std::endl;
        std::cout << "piece is: " << std::endl;
        printList(piece);
    }
    if (piece.size() == 0) {
        return piece;
    }

    int nx = voxel_list->m_dimX;
    int ny = voxel_list->m_dimY;
    int nz = voxel_list->m_dimZ;
    int size = nx*ny*nz;

    bool *visited = new bool[size];
    bool *inPiece = new bool[size];
    bool connected = false;

    std::list<Voxel> newQueue;
    Voxel start = piece[0];
    Voxel current;
    bool in;
    Voxel end;
    std::vector<Voxel> path;
    std::vector<Voxel> neighbors;
    std::vector<Voxel> disconnected;
    //while (!connected) {
        for(unsigned int i=0; i<size; ++i) {
            visited[i] = false;
            inPiece[i] = false;
        }
        for (int i = 0; i < piece.size(); i++) {
            inPiece[piece[i].z*(ny*nx) + piece[i].y*ny + piece[i].x] = true;
        }
        newQueue.clear();
        
        //Mark the current node as visited and enqueue it
        visited[start.z*(nx*ny) + start.y*ny + start.x] = true;
        newQueue.push_back(start);
        if (debug) {
            std::cout << "checking connection" << std::endl;
        }
        while ( !newQueue.empty() ) {
            current = newQueue.front();

            newQueue.pop_front();
            neighbors = getNeighbors(current, voxel_list, 1);
            if (debug) {
                std::cout << "current is " << current.toString() << ", it's neighbors are:" << std::endl;
                printList(neighbors);
            }

            for (int i = 0; i < neighbors.size(); i++) {
                if ( !visited[ neighbors[i].z*(nx*ny) + neighbors[i].y*ny + neighbors[i].x ] && inPiece[neighbors[i].z*(ny*nx) + neighbors[i].y*ny + neighbors[i].x]  ) {
                    if (debug) {
                        std::cout << "REACHED " << neighbors[i].toString() << " FROM " << current.toString() << std::endl;
                    }
                    visited[ neighbors[i].z*(nx*ny) + neighbors[i].y*ny + neighbors[i].x ] = true;
                    newQueue.push_back(neighbors[i]);
                }
            }
        }
        if (debug) {
            std::cout << "thru there" << std::endl;
        }
        // okay so now make sure that all pieces have been visited
        disconnected.clear();
        for (int i = 0; i < piece.size(); i++) {
            if (!visited[piece[i].z*(ny*nx) + piece[i].y*ny + piece[i].x]) {
                disconnected.push_back(piece[i]);
            } else {
                if (debug) {
                    std::cout << "visited " << piece[i].toString() << " somewhere" << std::endl;
                }
            }

        }

        // GUCCI
        if (disconnected.size() == 0) {
            connected = true;
        }
        
        if (debug) {
            std::cout << "now, finding paths " << std::endl;
        }
        path.clear();
        for (int i = 0; i < disconnected.size(); i++) {
            if (debug) {
                std::cout << "\tdelinquent:" <<  start.toString() << ", path is (starting from " << disconnected[i].toString()<< "):" << std::endl;
            }
            path = shortestPathThree(voxel_list, disconnected[i], piece);
            if (debug) {
                printList(path);
            }
            for (int j = 0; j < path.size(); j++) {
                end = path[j];
                end += normal;
                while (end.x < nx && end.x > -1 && end.y > -1 && end.y < ny && end.z > -1 && end.z < nz) {
                    if ( voxel_list->isInside(end.x, end.y, end.z) == 1) {
                        in = false;
                        for (int k = 0; k < piece.size(); k++) {
                            if (piece[k] == end) {
                                in = true;
                            }
                        }
                        if (!in) {
                            if (debug) {
                                std::cout << "adding to piece " << end.toString() << std::endl;
                            }
                            piece.push_back(end);
                        } else {
                            if (debug) {
                                std::cout << end.toString() << " is already in" << std::endl;
                            }
                        }
                    }
                    end += normal;
                }
            }   
        }
    //}
    if (debug) {
        std::cout << "escaped ensurePieceConnectivity" << std::endl;
    }
    return piece;
}

/**
    Ensures that a piece is connected to itself during partition process.

    @param voxel_list A VoxelGrid representing the current state of the puzzle.
    @param piece The piece being partitioned.
    @param pieceId The ID of the piece as it's set to in voxel_list
    @return true if the piece is connected, false otherwise.
*/
bool checkPieceConnectivity(CompFab::VoxelGrid * voxel_list, std::vector<Voxel> piece, int pieceId) {
    if (debug) {
        std::cout << "in checkPieceConnectivity" << std::endl;
    }
    bool connected = true;
    int nx = voxel_list->m_dimX;
    int ny = voxel_list->m_dimY;
    int nz = voxel_list->m_dimZ;
    int size = nx*ny*nz;
    
    bool *visited = new bool[size];
    for (int i = 0; i < size; i++) {
        visited[i] = false;
    }
    std::list<Voxel> queue;
    std::vector<Voxel> neighbors;
    Voxel current;
    
    Voxel start = Voxel(-1,-1,-1);
    for (int i = 0; i < piece.size(); i++) {
        if (voxel_list->isInside(piece[i].x, piece[i].y, piece[i].z) == pieceId) {
            start = piece[i];
            break;
        }
    }
    if (start == Voxel(-1,-1,-1)) {
        std::cout << "Error in checkPieceConnectivity" << std::endl;
    }
            
    //Mark the current node as visited and enqueue it 
    visited[start.z*(nx*ny) + start.y*ny + start.x] = true;
    queue.push_back(start);

    while ( !queue.empty() ) {
        current = queue.front();
        queue.pop_front();
        neighbors = getNeighbors(current, voxel_list, pieceId);
        for (int i = 0; i < neighbors.size(); i++) {
            if ( !visited[ neighbors[i].z*(nx*ny) + neighbors[i].y*ny + neighbors[i].x ] ) {
                visited[ neighbors[i].z*(nx*ny) + neighbors[i].y*ny + neighbors[i].x ] = true;
                queue.push_back(neighbors[i]);
            }
        }
    }
    for (int i = 0; i < piece.size(); i++) {
        if (voxel_list->isInside(piece[i].x, piece[i].y, piece[i].z) == pieceId && !visited[piece[i].z*(ny*nx) + piece[i].y*ny + piece[i].x]) {
            connected = false;
            break;
        }
    }
    return connected;
}

/**
    Partitions a piece into 2 pieces.

    @param voxel_list A VoxelGrid representing the current state of the puzzle.
    @param scores An AccessibilityStruct representing the current accesibility scores of the puzzle.
    @param piece The piece being partitioned.
    @param numPartition The piece ID as set in voxel_list
    @param pieceSize The size of the partition
    @return The partitioned piece.
*/
std::vector<Voxel> partitionPiece(CompFab::VoxelGrid * voxel_list, AccessibilityGrid * scores, std::vector<Voxel> piece, int numPartition, int pieceSize) {
    //if (debug) {
        std::cout << "in partitionPiece" << std::endl;
        std::cout << "piece size is " << std::to_string(pieceSize) << std::endl;
    //}
    int nx = voxel_list->m_dimX;
    int ny = voxel_list->m_dimY;
    int nz = voxel_list->m_dimZ;
    int size = nx*ny*nz;
    
    //first, sort piece by accessibility
    std::vector<VoxelSort> sorted;
    for (int i = 0; i < piece.size(); i++) {
        sorted.push_back(VoxelSort(piece[i], scores->score(piece[i].x, piece[i].y, piece[i].z)));
    }
    std::sort (sorted.begin(), sorted.end(), voxelSortSorter);
    piece.clear();
    for (int i = 0; i < sorted.size(); i++) {
        piece.push_back(sorted[i].voxel);
    }

    bool *visited = new bool[size];
    for (int i = 0; i < size; i++) {
        visited[i] = false;
    }
    std::list<Voxel> queue;
    std::vector<Voxel> neighbors;
    Voxel current;

    Voxel start = piece[0];
    //Mark the current node as visited and enqueue it
    visited[start.z*(nx*ny) + start.y*ny + start.x] = true;
    queue.push_back(start);

    std::vector<Voxel> partition;
    std::vector<Voxel> temp;
    while ( !queue.empty() ) {
        std::cout << "partition size is " <<  std::to_string(partition.size()) << std::endl;
        current = queue.front();
        if (partition.size() >= pieceSize ) {
            break;
        }

        queue.pop_front();
        neighbors = getNeighbors(current, voxel_list, numPartition);
        for (int i = 0; i < neighbors.size(); i++) {
            if ( !visited[ neighbors[i].z*(nx*ny) + neighbors[i].y*ny + neighbors[i].x ] ) {
                // Ensure adding piece doesn't disconnect the partitions
                temp = partition;
                temp.push_back(neighbors[i]);
                for (int j = 0; j< temp.size(); j++) {
                    voxel_list->isInside(temp[j].x, temp[j].y, temp[j].z) = 0;
                }
                if (checkPieceConnectivity(voxel_list, piece, numPartition)) {
                    partition = temp;
                }
                for (int j = 0; j< temp.size(); j++) {
                    voxel_list->isInside(temp[j].x, temp[j].y, temp[j].z) = numPartition;
                }
                visited[ neighbors[i].z*(nx*ny) + neighbors[i].y*ny + neighbors[i].x ] = true;

                queue.push_back(neighbors[i]);
            }
        }
    }
    delete[] visited;
    return partition;   
}
