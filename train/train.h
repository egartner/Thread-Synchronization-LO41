/* Train.h
----------------
Projet LO41
Etienne Gartner*/

typedef enum Sens Sens;
enum Sens {EST, OUEST};

typedef enum Type Type;
enum Type {TGV, GL, M};

typedef enum Pos Pos;
enum Pos {POS_EST, GARE, VOIE_M, AIGUILLAGE, GARAGE, TUNNEL, VOIES, POS_OUEST};

typedef struct Train Train;
struct Train{
        int id;
        Sens sens;
        Type type;
	Pos pos;
};

void * main_train(void *arg);
void toString (Train train);
void gare (Train train);
void voie (Train train);
void aiguillage_P0(Train train);
void aiguillage_P1(Train train);
void tunnel (Train train);
