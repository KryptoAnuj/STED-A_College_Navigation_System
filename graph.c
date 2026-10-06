#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LOCATIONS 200
#define MAX_BUILDINGS 20

typedef enum
{
    ROOM,CORRIDOR,STAIRCASE,LIFT,ENTRANCE

} LocationType;
typedef struct Edge
{
    int destination;
    int distance;
    struct Edge *next;
} Edge;
typedef struct
{
    int id;
    char name[50];
} Building;
typedef struct
{
    int id;
    char name[50];
    int buildingID;
    float x;
    float y;
    float z;
    LocationType type;
} Location;
typedef struct
{
    int numberOfLocations;
    int numberOfBuildings;
    Building buildings[MAX_BUILDINGS];
    Location locations[MAX_LOCATIONS];
    // Adjacency List
    Edge *adjList[MAX_LOCATIONS];
} Graph;
void initializeGraph(Graph *graph)
{
    graph->numberOfLocations = 0;
    graph->numberOfBuildings = 0;
    for (int i = 0; i < MAX_LOCATIONS; i++)
    {
        graph->adjList[i] = NULL;
    }
}
void addBuilding(Graph *graph, int id, const char *name)
{
    graph->buildings[id].id = id;
    strcpy(graph->buildings[id].name, name);
    graph->numberOfBuildings++;
}
// ======================================================
// ADD LOCATION
// ======================================================
void addLocation(
    Graph *graph,
    int id,
    const char *name,
    int buildingID,
    LocationType type,
    float x,
    float y,
    float z
)
{
    graph->locations[id].id = id;
    strcpy(graph->locations[id].name, name);
    graph->locations[id].buildingID = buildingID;
    graph->locations[id].type = type;
    graph->locations[id].x = x;
    graph->locations[id].y = y;
    graph->locations[id].z = z;
    graph->numberOfLocations++;
}
void addPath(
    Graph *graph,
    int source,
    int destination,
    int distance
)
{
    Edge *newEdge = malloc(sizeof(Edge));
    if (newEdge == NULL)
    {
        printf("Memory allocation failed!\n");
        return;
    }
    newEdge->destination = destination;
    newEdge->distance = distance;
    newEdge->next = graph->adjList[source];
    graph->adjList[source] = newEdge;
    newEdge = malloc(sizeof(Edge));
    if (newEdge == NULL)
    {
        printf("Memory allocation failed!\n");
        return;
    }
    newEdge->destination = source;
    newEdge->distance = distance;
    newEdge->next = graph->adjList[destination];
    graph->adjList[destination] = newEdge;
}
const char *getLocationType(LocationType type)
{
    switch (type)
    {
        case ROOM:
            return "Room";

        case CORRIDOR:
            return "Corridor";

        case STAIRCASE:
            return "Staircase";

        case LIFT:
            return "Lift";

        case ENTRANCE:
            return "Entrance";

        default:
            return "Unknown";
    }
}
void displayBuildings(Graph *graph)
{
    printf("              BUILDINGS\n");
    for (int i = 0; i < graph->numberOfBuildings; i++)
    {
        printf("\nID   : %d",
               graph->buildings[i].id);
        printf("\nName : %s\n",
               graph->buildings[i].name);
    }
}
void displayLocations(Graph *graph)
{
    printf("CAMPUS LOCATIONS\n");
    for (int i = 0; i < graph->numberOfLocations; i++)
    {
        Location *location = &graph->locations[i];

        printf("\n----------------------------------------");

        printf("\nID       : %d",
               location->id);

        printf("\nName     : %s",
               location->name);

        printf("\nBuilding : %s",
               graph->buildings[
                   location->buildingID
               ].name);

        printf("\nType     : %s",
               getLocationType(location->type));

        printf("\nCoordinates : (%.1f, %.1f, %.1f)",
               location->x,
               location->y,
               location->z);

        printf("\n");
    }
}
void displayGraph(Graph *graph)
{
    printf("ADJACENCY LIST\n");
    for (int i = 0; i < graph->numberOfLocations; i++)
    {
        printf("\n%s",
               graph->locations[i].name);

        Edge *current = graph->adjList[i];

        while (current != NULL)
        {
            printf(" -> %s (%dm)",
                   graph->locations[
                       current->destination
                   ].name,
                   current->distance);

            current = current->next;
        }

        printf("\n");
    }
}
void freeGraph(Graph *graph)
{
    for (int i = 0;
         i < graph->numberOfLocations;
         i++)
    {
        Edge *current = graph->adjList[i];

        while (current != NULL)
        {
            Edge *temp = current;
            current = current->next;
            free(temp);
        }
        graph->adjList[i] = NULL;
    }
}
// MAIN
int main()
{
    Graph campus;
    initializeGraph(&campus);
    addBuilding(
        &campus,0,"CSE Building");
    addBuilding(
        &campus,1,"Main Academic Block");
    addBuilding(
        &campus,2,"Library");
    addBuilding(
        &campus,3,"Administrative Block");
    addLocation(
        &campus,0,"CSE Entrance",0,ENTRANCE,
        0.0, 0.0, 0.0);
    addLocation(
        &campus, 1,"CR15", 0, ROOM,
        0.0, 0.0, 0.0 );
    addLocation(
        &campus,2,"CR16", 0,ROOM,
        0.0, 0.0, 0.0 );
    addLocation(
        &campus,3,"Staircase",0,STAIRCASE,
        0.0, 0.0, 0.0);
    addLocation(
        &campus,4,"LT12",0,ROOM,
        0.0, 0.0, 0.0);
    addLocation(
        &campus,5,"LT13",0,ROOM,
        0.0, 0.0, 0.0);
    addLocation(
        &campus,6,"Academic Entrance",1,ENTRANCE,
        0.0, 0.0, 0.0);
    addLocation(
        &campus,7,"CR20",1,ROOM,
        0.0, 0.0, 0.0);
    addLocation(
        &campus,8,"Library Entrance",2,ENTRANCE,
        0.0, 0.0, 0.0);
    addLocation(
        &campus,9,"Admin Entrance",3,ENTRANCE,
        0.0, 0.0, 0.0);
    addPath(
        &campus,0,1,10);
    addPath(
        &campus,1,3,15);
    addPath(
        &campus,3,2,10);
    addPath(
        &campus,3,4,12);
    addPath(
        &campus,4,5,10);
    addPath(
        &campus,0,6,150);
    addPath(
        &campus,6,7,20);
    addPath(
        &campus,6,8,100);
    addPath(
        &campus,6,9,200);
    displayBuildings(&campus);
    displayLocations(&campus);
    displayGraph(&campus);
    freeGraph(&campus);
    return 0;
}