/*----------------------------------------------------------------------*/
/*                                                                      */
/*  File    :   CHIMAERA.H                                              */
/*  Author  :   Chris Newall                                            */
/*  Date    :   31st January 2002                                       */
/*  Function:   Declarations and function prototypes for the infinite   */
/*              adventure CHIMAERA.C                                    */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  ***  Revision History  ***                                          */
/*                                                                      */
/*  12 May 2003 Version C1.001A  Amended version code                   */
/*                                                                      */
/*---- Session logging -------------------------------------------------*/
#define LOGGING    1   /*  Logging to file chimaera.log on=1, off=0     */

/*---- System dependent settings ---------------------------------------*/
#define DISPWIDTH 80   /*  Display width for text wrapping              */
#define DELAY 1000  /*  sleep() delay - change to suit processor     */

/*---- Version information ---------------------------------------------*/
static char version[] = { "Version C1.001A" };

/*---- Define array dimensions -----------------------------------------*/
#define MAXACT    81  /*  Current number of actions                     */
#define MOVMOD    60  /*  Current value of action MODULUS               */
#define MAXOBJ    47  /*  Number of objects in database                 */
#define MAXHINT   31  /*  Number of hints[] available                   */
#define MAXEXP    13  /*  Number of expletives                          */
#define MAXFLAG   51  /*  Dimension of FLAGS array                      */
#define MAXVAL    51  /*  Dimension of VALUES array                     */
#define MAXMON     9  /*  Number of monsters                            */
#define MAXPSEU   11  /*  Number of pseudo objects                      */
#define MAXREST  100  /*  Must be larger than MAXFLAG, MAXVAL or MAXOBJ */
#define MAXDISPLAY 2048 /* Size of showtext() display buffer            */

/*---- Define parameters -----------------------------------------------*/

/* Index pointers to objects                                            */
#define FOOD        1   /*  Food                                        */
#define BOTTLE      2   /*  Bottle                                      */
#define LAMP        3   /*  Lamp                                        */
#define PLANT       4   /*  Plant                                       */
#define KEYS        5   /*  Keys                                        */
#define SWORD       6   /*  Sword                                       */
#define ROD         7   /*  Rod                                         */
#define ROPE        8   /*  Rope                                        */
#define GARLIC      9   /*  Garlic                                      */
#define STAFF      10   /*  Staff                                       */
#define BASKET     11   /*  Basket                                      */
#define PEARL      12   /*  Pearl                                       */
#define COINS      13   /*  Coins                                       */
#define NUGGET     14   /*  Nugget                                      */
#define SILVER     15   /*  Silver                                      */
#define DIAMOND    16   /*  Diamond                                     */
#define JEWELS     17   /*  Jewels                                      */
#define VASE       18   /*  Vase                                        */
#define CARPET     19   /*  Carpet                                      */
#define BANKNOTES  20   /*  Banknotes                                   */
#define MANUSCRIPT 21   /*  Manuscript                                  */
#define FLUTE      22   /*  Flute                                       */
#define BOOK       23   /*  Book                                        */
#define MUSIC      24   /*  Music                                       */
#define VIOLETS    25   /*  Violets                                     */
#define BOX        26   /*  Box                                         */
#define PYRAMID    27   /*  Pyramid                                     */
#define CUSHION    28   /*  Cushion                                     */
#define DAGGER     29   /*  Dagger                                      */
#define GOBLET     30   /*  Goblet                                      */
#define SAPPHIRE   31   /*  Sapphire                                    */
#define NECKLACE   32   /*  Necklace                                    */
#define SCEPTRE    33   /*  Sceptre                                     */
#define TREASURE   34   /*  Treasure                                    */
#define RUBY       35   /*  Ruby                                        */
#define CHARM      36   /*  Charm                                       */
#define BRACELET   37   /*  Bracelet                                    */
#define OYSTER     38   /*  Oyster                                      */
#define CLAM       39   /*  Clam                                        */
#define ELIXIR     40   /*  Elixir of life                              */
#define MIRROR     41   /*  Mirror                                      */
#define BRICKBATS  42   /*  Brickbat                                    */
#define GNOME      43   /*  Gnome                                       */
#define SERPENTINE 44   /*  Serpentine                                  */
#define VENUS      45   /*  Venus de Milo                               */
#define GOLDRING   46   /*  Golden ring                                 */

/* Parameters for moves                                                 */
#define NORTH 1
#define SOUTH 2
#define EAST  3
#define WEST  4
#define SE    5
#define SW    6
#define NE    7
#define NW    8
#define UP    9
#define DOWN 10
#define IN   11
#define OUT  12

/* Index pointers to values[]                                           */
#define BASE      6      /*  Basecamp location                          */
#define FIRSTX    7      /*  Base X co-ordinate                         */
#define FIRSTY    8      /*  Base Y co-ordinate                         */
#define LASTX     9      /*  Last X co-ordinate                         */
#define LASTY    10      /*  Last Y co-ordinate                         */
#define LASTZ    11      /*  Last Z co-ordinate                         */
#define LUCK     12      /*  Players luck quota                         */
#define MONTH    13      /*  Index to months[] and mdays[]              */
#define DAY      14      /*  The day of the month                       */
#define WEEKDAY  15      /*  Index to weekdays[]                        */
#define DAWN     16      /*  Index to dawn[]                            */
#define DUSK     17      /*  Index to dusk[]                            */
#define DAYTIME  18      /*  Time of day                                */

#define HELD     20      /*  Number of things he holds                  */
#define LIGHT    21      /*  Amount of light left in lamp               */
#define POOL     22      /*  Water pool                                 */
#define MAGONE   23      /*  Times he has used first magic word         */
#define DWFNUM   24      /*  Total number of dwarves                    */
#define DWFNOW   25      /*  Number of active dwarves                   */
#define MONCOUNT 26      /*  Number of active monsters                  */
#define GORLOC   27      /*  Location of gorgon                         */
#define SEED     28      /*  Seed for rand()                            */
#define BRKBOT   29      /*  Broken glass from bottle                   */
#define BRKVAS   30      /*  Broken glass from vase                     */
#define BRKGOB   31      /*  Broken glass from goblet                   */
#define BRKMIR   32      /*  Broken glass from mirror                   */
#define THIRST   33      /*  Thirst rating                              */
#define FLICK    34      /*  Flickering lamp count                      */
#define RNDCOUNT 35      /*  Number of calls to rnd(n)                  */
#define HINTPTR  36      /*  Index to relevant hints[]                  */
#define RUNOUT   37      /*  Value of moveno when someone runs out      */

/* Index pointers to animals and monsters                               */
#define LEOPARD     1
#define TIGER       2
#define LION        3
#define LYNX        4

#define ANTELOPE    1
#define WILDEBEEST  2
#define ZEBRA       3
#define GAZELLE     4
#define DEER        5
#define ELEPHANT    6
#define BUFFALO     7

#define BATS        1
#define DWARF       2
#define SNAKE       3
#define GORGON      4
#define ELF         5
#define TROLL       6
#define DRAGON      7
#define VAMPIRE     8

/* Index pointers to flags[]                                            */
#define WIZARD     1      /* Wizard flag 0=No 1=Yes                     */
#define NIGHT      2      /* Night flag                                 */
#define DARK       3      /* Dark here flag                             */
#define GLOW       4      /* Underground glow flag                      */
#define LAMPON     5      /* Lamp on flag                               */
#define WAVER      6      /* Glow (if any) wavers                       */
#define FR13       7      /* Friday 13th flag                           */
#define HALLOW     8      /* Halloween flag                             */
#define LIFE       9      /* Life (or death) flag                       */

#define BOXLOK    10      /* Box locked flag                            */
#define DORLOK    11      /* Door locked flag                           */

#define ROLING    12      /* Rolling grass desc. flag                   */
#define HIGRAS    13      /* High grass flag                            */
#define ANIMAL    14      /* Animals in sight flag                      */
#define HERD      15      /* Herd of animals flag                       */
#define CARNIV    16      /* Carnivores flag                            */
#define SEEN      17      /* Seen by carnivores flag                    */

#define DDOT      18      /* Full Stop flag                             */
#define TREE      19      /* Tree in sight flag                         */
#define NOTUP     20      /* Not up tree flag                           */
#define HOLE      21      /* Hole in sight flag (Not used)              */
#define HOME      22      /* At base camp flag                          */
#define INTENT    23      /* In the tent flag                           */

#define SMALL     24      /* Small location flag                        */
#define LARGE     25      /* Large location flag                        */
#define SWITCH    26      /* Flip flop flag                             */
#define LEAD      27      /* Lead(s/ing) flag                           */
                          
#define UNDEAD    28      /* Player undead flag                         */
#define GENIE     29      /* Genie flag                                 */
#define IMMORTAL  30      /* Drunk elixir of life                       */
#define EMPTY     31      /* Water bottle empty flag                    */
#define DRIP      32      /* Water drips                                */

#define COPRNT    33      /* Flag for printing coordinates              */
#define SHAFT     34      /* Underground shaft flag                     */

#define HANDSFULL 35      /* Hands full, can't carry any more flag      */
#define AUTOHINT  36      /* Hint status -1=initial state 0=off 1=auto  */
#define RABBIT    37      /* White rabbit can appear                    */
#define EXCALIBER 38      /* True for first sword until thrown          */
#define RINGON    39      /* Wearing ring on finger                     */
#define SHANGRI   40      /* Shangri La visit                           */
#define OVERDRAWN 41      /* Account overdrawn                          */

/*---- Declare global variables arrays and structures ------------------*/

static FILE *log_file;

static int x, y, z;                    /* Location co-ordinates         */
static int here;                       /* This is where we are          */
static int described;                  /* Displayed description flag    */

static int xdist,											 /* EW distance from base camp    */
       ydist,                          /* NS distance from base camp    */
       flagxy,                         /* xdist * ydist                 */
       distance;                       /* Distance from base camp       */

static int pseudorand;            /* Psuedo random number from rnd()    */

static int  objloc[MAXOBJ];       /* Object locations                   */
static int  points[MAXOBJ];       /* Object scores                      */
static int  ipt[MAXOBJ];          /* Pointers for random access         */
static int  obhere[MAXOBJ];       /*  Objects (not) here                */
static int  secure[MAXOBJ];       /*  Objects in tent                   */
static int  gone[MAXOBJ];         /*  Objects vanished for ever         */

static int moveno;         /* Move number                               */
static int score;          /* Current score excluding objects in tent   */
static int delay;          /* Arguement for time delay function         */
static int helpno;         /* No of times help has been requested       */
static int advno;          /* The Adventure number                      */
static int cow, cat;       /* Indexes to cows[] and cats[]              */ 

static char userpass[21];  /* User supplied password                    */

static char inbuff[81];                /* Input buffer                  */
static char display[MAXDISPLAY];       /* Buffer for terminal output    */
static char action[11];                /* Action verb from parse()      */
static char object[11];                /* Object noun from parse()      */
static char lastaction[11];            /* Last action verb              */
static char lastobject[11];            /* Last object noun              */

static int ways[15];                   /* Current moves No=0 Yes=1      */         
static long values[MAXVAL];            /* Integer values                */
static int monstr[MAXMON];             /* Monster action values         */
static int flags[MAXFLAG];             /* Logical flags                 */

/* Action verbs [n.b. words in lower case are not implemented yet]      */
/* n.b. Verbs higher than index 60 are alternative command words        */
static char dicact[MAXACT][10] = 
   {"",                                        /* dicact[0] is reserved */
    "N",         "S",         "E",         "W",         "NE",
    "NW",        "SE",        "SW",        "UP",        "DOWN",     /*10*/
    "IN",        "OUT",       "ON",        "TAKE",      "KILL",
    "free",      "LOOK",      "CHASE",     "SHANGRI",   "",         /*20*/
    "EAT",       "WIZARD",    "go",        "THROW",     "WAVE",
    "STAMP",     "PUT",       "DROP",      "OFF",       "RUB",      /*30*/
    "FILL",      "PLAY",      "READ",      "FIND",      "1$$$$",
    "INVENTORY", "HELP",      "SCORE",     "QUIT",      "DRINK",    /*40*/
    "OPEN",      "CLOSE",     "LOCK",      "UNLOCK",    "CALL",
    "INSTRUCT",  "PLUGH",     "GAMIC",     "SLEEP",     "TIME",     /*50*/
    "WHERE",     "ADVENTURE", "EXAMINE",   "HINT",      "",
    "",          "",          "SAVE",      "RESTORE",   "",         /*60*/
    "NORTH",     "SOUTH",     "EAST",      "WEST",      "NORTHEAST",
    "NORTHWEST", "SOUTHEAST", "SOUTHWEST", "CLIMB",     "DESCEND",  /*70*/
    "ENTER",     "EXIT",      "LIGHT",     "GET",       "SLAY",
    "release",   "DESCRIBE",  "FOLLOW",    "",          ""          /*80*/
   };

/* ----------------- Declare the current lexicon -----------------------*/
/* Descriptive verbs */
#define MAXVERB 8
static char verbs[MAXVERB][20] =
   {"",                                         /* verbs[0] is reserved */
    "standing in",
    "lying in",
    "walking through",
    "walking along",
    "crawling through",
    "crawling along",
    "climbing"
   };

#define MAXNOUN 19                   /* Descriptive nouns */
static char nouns[MAXNOUN][20] =
   {"",                                         /* nouns[0] is reserved */
    "oubliette",
    "crypt",
    "dungeon",
    "cell",
    "alcove",
    "cavern",
    "grotto",
    "cave",
    "tunnel",
    "antechamber",
    "passageway",
    "hall",
    "corridor",
    "chamber",
    "passage",
    "room",
    "well",
    "shaft"
   };

#define MAXADJ1 17                    /* Descriptive adjectives */
static char adject1[MAXADJ1][20] =
   {"",                                       /* adject1[0] is reserved */
    "an impressive",
    "an ornate",
    "an arched",
    "an imposing",
    "a cramped",
    "a gigantic",
    "a tiny",
    "an enormous",
    "a low",
    "a high",
    "a narrow",
    "a wide",
    "a little",
    "a big",
    "a small",
    "a large"
   };
#define MAXADJ2 13                   /* Qualifying adjectives */
static char adject2[MAXADJ2][20] =
   {"",                                       /* adject2[0] is reserved */
    " evil",
    " smelly",
    "",
    " ominous",
    "",
    " dank",
    "",
    " sandy",
    "",
    " damp",
    "",
    " dry"
   };

static char thing[MAXOBJ][12] =   /* Object names */
   {"",                                        /* thing[0] is reserved */
    "FOOD",       "BOTTLE",    "LAMP",    "PLANT",      "KEYS",      /*  5 */
    "SWORD",      "ROD",       "ROPE",    "GARLIC",     "STAFF",     /* 10 */
    "BASKET",     "PEARL",     "COINS",   "NUGGET",     "SILVER",    /* 15 */
    "DIAMOND",    "JEWELS",    "VASE",    "CARPET",     "BANKNOTES", /* 20 */
    "MANUSCRIPT", "FLUTE",     "BOOK",    "MUSIC",      "VIOLETS",   /* 25 */
    "BOX",        "PYRAMID",   "CUSHION", "DAGGER",     "GOBLET",    /* 30 */
    "SAPPHIRE",   "NECKLACE",  "SCEPTRE", "TREASURE",   "RUBY",      /* 35 */
    "CHARM",      "BRACELET",  "OYSTER",  "CLAM ",      "ELIXIR",    /* 40 */
    "MIRROR",     "BRICKBATS", "GNOME",   "SERPENTINE", "VENUS",     /* 45 */
    "RING"
   };

static char thingpref[MAXOBJ][6] =  /* Prefixes for oject names */
   {"",                                        /* thingpref[0] is reserved */
    "some ",  /*  1 */
    "a ",
    "a ",
    "a ",
    "some ",  /*  5 */
    "a ",
    "a ",
    "some ",
    "some ",
    "a ",     /* 10 */
    "a ",
    "a ",
    "some ",
    "a ",
    "some ",  /* 15 */
    "a ",
    "some ",
    "a ",
    "a ",
    "some ",  /* 20 */
    "a ",
    "a ",
    "a ",
    "some ",
    "some ",  /* 25 */
    "a ",
    "a ",
    "a ",
    "a ",
    "a ",     /* 30 */
    "a ",
    "a ",
    "a ",
    "some ",
    "a ",     /* 35 */
    "a ",
    "a ",
    "an ",
    "a ",
    "the ",   /* 40 */
    "a ",
    "some ",
    "a ",
    "a ",
    "a ",     /* 45 */
    "a "
   };

static char thingdesc[MAXOBJ][40] =   /* Object descriptions */
   {"",                                        /* thingdesc[0] is reserved */
    "some tasty food",                            /*  1 */
    "a green glass bottle",
    "a tarnished brass lamp",
    "a little plant",
    "a bunch of keys",                            /*  5 */
    "a rusty bloodstained sword",
    "a polished black rod",
    "a coil of rope",
    "a clove of garlic",
    "a long wooden staff",                        /* 10 */
    "a small wicker basket",
    "a glistening pearl",
    "a handful of coins",
    "a golden nugget",
    "several bars of silver",                     /* 15 */
    "a brilliant diamond",
    "some precious jewels",
    "a delicate Ming vase",
    "an ornate Persian carpet",
    "some bundles of banknotes",                  /* 20 */
    "an illuminated manuscript",
    "a beautiful silver flute",
    "a dog-eared book",
    "some sheets of music",
    "a bunch of violets",                         /* 25 */
    "a gem encrusted box",
    "a platinum pyramid",
    "a richly embroidered cushion",
    "a jewelled dagger",
    "a crystal goblet",                           /* 30 */
    "a priceless sapphire",
    "a glittering necklace",
    "a princely sceptre",
    "a small chest of treasure",
    "a blood-red ruby",                           /* 35 */
    "a good luck charm",
    "an emerald bracelet",
    "an enormous oyster",
    "a giant clam",
    "the elixir of life",                         /* 40 */
    "a highly polished mirror",
    "some scattered brickbats",
    "a pottery gnome",
    "a block of serpentine",
    "a statue of the Venus de Milo",              /* 45 */
    "a beautiful golden ring"
   };

static char pseudo[MAXPSEU][8] =   /* Pseudo objects */
   {"",                                        /* pseudo[0] is reserved */
    "camp","tent","base","water","servant",
    "","","","",""
   };

static char explet[MAXEXP][10] =
   {"",                                        /* explet[0] is reserved */
    "BLAST","BUGGER","DAMN","SHIT","SOD",
    "HELL","FUCK","PISS","WANKER","BOLLOCKS",
    "CUNT","ARSEHOLE"
   };

static char months[13][10] =     /* Names of months */
   {"",                                        /* months[0] is reserved */
    "January","February","March","April",
    "May","June","July","August",
    "September","October","November","December"
   };


static int mdays[13] =      /* Number of days in each month */
   {
    0,31,28,31,30,31,30,31,31,30,31,30,31
   };

static int dawn[13] =       /* Time of dawn                 */
   {
    0,17,15,12,9,7,6,7,9,12,15,17,18
   };

static int dusk[13] =       /* Time of dusk                 */
   {
    0,31,33,36,39,41,42,41,39,36,33,31,30
   };

static char weekdays[8][10] =    /* Names of weekdays */
   {"",                                      /* weekdays[0] is reserved */
    "Monday","Tuesday","Wednesday","Thursday","Friday","Saturday","Sunday"
   };

static char cows[8][12] =        /* Herbivors */
   {"*",                                          /* cows[0] is reserved */
    "antelopes","wildebeest","zebras","gazelles",
    "deer","elephants","buffalos"
   };

static char cats[5][8] =          /* Carnivors */
   {"*",                                          /* cats[0] is reserved */
    "leopard","tiger","lion","lynx"
   };

static char beasts[MAXMON][10] =       /* Monsters */
   {"",                                        /* beasts[0] is reserved */
    "BAT","DWARF","SNAKE","GORGON",
    "ELF","TROLL","DRAGON","VAMPIRE"
   };

static char classes[11][20] = 
      {"",                                   /* classes[0] is reserved */
       "a novice","an apprentice","a student","a graduate","a third class",
       "a second class","a first class","a master","a grand master","a supreme champion"
      };

static char hints[MAXHINT][81] =       /* Hints */
   {
    "",                                      /* Hint[0] is reserved */
    "The only really safe place to sleep is in your tent",                      /*  1 */
    "The carnivores may attack at any time once they have seen you",            /*  2 */
    "Your luck will run out if you go for too long without a drink",            /*  3 */
    "",      /*  4 */
    "",      /*  5 */
    "",      /*  6 */
    "",      /*  7 */
    "",      /*  8 */
    "",      /*  9 */
    "",      /* 10 */
    "",
    "",
    "",
    "",
    "", /* 15 */
    "",
    "",
    "",
    "",
    "", /* 20 */
    "",
    "",
    "",
    "",
    "", /* 25 */
    "",
    "",
    "",
    "",
    "", /* 30 */
   };

static int hintsdone[MAXHINT];

/* X, Y and Z increments and waybits */
static int xinc[11] = {0,0,0,1,-1,1,-1,1,-1,0,0};
static int yinc[11] = {0,1,-1,0,0,1,1,-1,-1,0,0};
static int zinc[11] = {0,0,0,0,0,0,0,0,0,-1,1};
static int waybit[11] = {0,8,8,16,16,32,64,64,32,128,128};

static char routes[9][12] =     /* Route nouns */
   {"",                                        /* routes[0] is reserved */
    "north","south","east","west",
    "northeast","northwest","southeast","southwest"
   };

static char numerals[6][6] =      /* Number nouns */
   {
    "zero","one","two","three","four","five"
   };

static char times[49][10] =  /* Times of day for gettime() function */
   {"",                      /* times[0] is reserved                */
    "Midnight","00.30","01.00","01.30","02.00","02.30",
    "03.00","03.30","04.00","04.30","05.00","05.30",
    "06.00","06.30","07.00","07.30","08.00","08.30",
    "09.00","09.30","10.00","10.30","11.00","11.30",
    "Midday","12.30","13.00","13.30","14.00","14.30",
    "15.00","15.30","16.00","16.30","17.00","17.30",
    "18.00","18.30","19.00","19.30","20.00","20.30",
    "21.00","21.30","22.00","22.30","23.00","23.30"
    };

/*==== Function Prototypes =============================================*/
int main         (void);                /* Main function                */
/*---- Utility Functions -----------------------------------------------*/
void describe    (int n);              /* Describe the current location */
int  my_getline     (void);                /* Get a line of input          */
void timdat      (int z);               /* Display the time and date    */
void initialise  (void);                /* Initialise arrays            */
int  instructions(int n);               /* User instructions            */
long locate      (int x, int y, int z); /* Calculate a location code    */
void locwun      (void);                /* Initialise object locations  */
void mon_start   (void);                /* Activate monsters            */
void monsters    (void);                /* Display active monsters      */
void move        (int n);               /* Make a move if possible      */
void news        (void);                /* Display Chimaera news        */
int  parse       (void);                /* Parse an input line          */
void pointr      (int n);               /* Random index creator         */
int  rnd         (int n);               /* Random number generator      */
void showthings  (void);                /* Show objects which are here  */
void showtext    (void);                /* Display the output line      */
void tnoua       (char *c);         /* Append to a line on the terminal */
void tnou        (char *c);             /* Output to user               */
void tnoint      (long n);          /* Append a long integer to line    */
void tnoulca     (char *c);             /* Append to line in lowercase  */
void tonl        (int n);               /* Output n blank lines         */
void welcome     (void);                /* Display opening screen       */
void worth       (void);                /* Initialise object values     */
int  yesno       (void);                /* Elicit a Yes or No answer    */
/*---- Miscellaneous Functions -----------------------------------------*/
void dead        (void);                /* The player has been killed   */
void newday      (void);                /* Increment the date           */
long plumb       (void);                /* Locate the bottom of a shaft */
void sleep       (int n);               /* Sleep for a while            */
void cheat       (void);                /* Display current settings     */
void swearbox    (char * token);        /* Fine for using a naughty word*/
/*---- Action Functions ------------------------------------------------*/
void callhim     (void);         /* Call someone                        */
void chase       (void);         /* Chase someone or something          */
void closeit     (int i);        /* Close something                     */
void drink       (int i);        /* Drink various things                */
void eat         (int i);        /* Eat various things                  */
void examine     (int i);        /* Examine something                   */
void fill        (int i);        /* Fill something                      */
void find        (int i);        /* Find something                      */
void take        (int i);        /* Take or get and object              */
void lockit      (int i);        /* Lock something                      */
void play        (int i);        /* Play an instrument                  */
void plugh       (void);         /* He says PLUGH                       */
void put         (int i, int j); /* Put down or drop something          */
void shangrila   (void);         /* Visit Shangri La                    */
void throw       (int i);        /* Throw something                     */
void gamic       (void);         /* Section in gamic language           */
void help        (void);         /* Help                                */
void hint        (char *c);      /* Context sensitive hints             */
void inventory   (void);         /* List the objects currently carried  */
void kill        (int i);        /* Kill something                      */
void look        (void);         /* Look about you                      */
void lucky       (void);         /* Luck messages                       */
void onlamp      (void);         /* Light the lamp                      */
void offlamp     (void);         /* Extinguish the lamp                 */
void openit      (int i);        /* Open something                      */
void quit        (int i);        /* Quit Chimaera                       */
void readit      (int i);        /* Read book, manuscript, music, etc.  */
void rub         (int i);        /* Rub the lamp and other things       */
void stamp       (int i);        /* Stamp out a dwarf                   */
void gettime     (int z);        /* Display the time                    */
void magic       (void);         /* The magic word was spoken           */
void scores      (void);         /* Display the current score           */
int  scorit      (void);         /* Return the current score            */
void slumber     (void);         /* Go to sleep                         */
void status      (void);         /* Establish a player's status         */
void adventure   (void);         /* Simulate the beginning of ADVENTURE */
void adv_plugh   (void);         /* Simulate PLUGH command in ADVENTURE */
void lookwhere   (char *c);      /* Direction and distance of base camp */
void calcdist    (void);         /* Calculate distance from base camp   */
void unknown     (void);         /* Response to an unrecognised command */
void unlock      (int i);        /* Unlock something                    */
void wave        (int i);        /* Wave the rod and other things       */
/*---- Save or restore a game ------------------------------------------*/
void savegame    (void);         /* Save a game                         */
void restore     (void);         /* Restore a game                      */
/*---- Get a user supplied password ------------------------------------*/
int get_password (void);         /* Get a user supplied password        */
/*---- End of File -----------------------------------------------------*/
