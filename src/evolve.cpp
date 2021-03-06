#include "PolyCell.h"
#include <tclap/CmdLine.h>
#include <unistd.h>

/*SodaPop

Copyright (C) 2017 Louis Gauthier

    SodaPop is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    
    SodaPop is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with SodaPop.  If not, see <http://www.gnu.org/licenses/>.
 */

int main(int argc, char *argv[])
{
    // these variables will hold the parameters input (or not) by the user
    int GENERATION_CTR = 1;
    int GENERATION_MAX = GENERATION_CTR + 1;
    int MUTATION_CTR = 0;
    double N=1;
    int DT = 1;
    double TIME = 0;
    char buffer[200];
    bool enableAnalysis = false;
    bool trackMutations = false;
    bool createPop = false;
    bool useShort = false;

    std::string inputType;
    std::string geneListFile, genesPath;
    std::string outDir, startSnapFile, matrixFile;

    auto rng = ProperlySeededRandomEngine();
    Ran rand_uniform(rng());                        
    uniformdevptr = &rand_uniform;

    // Wrap everything in a try block
    // errors in input are caught and explained to user
    try { 
    // Define the command line object
    TCLAP::CmdLine cmd("SodaPop: a multi-scale model of molecular evolution", ' ', "v1.0");

    // Define value arguments
    TCLAP::ValueArg<int> maxArg("m","maxgen","Maximum number of generations",false,10000,"int");
    TCLAP::ValueArg<int> popArg("n","size","Initial population size",false,1,"int");
    TCLAP::ValueArg<int> dtArg("t","dt","Time interval for snapshots",false,1,"int");

    //files
    TCLAP::ValueArg<std::string> prefixArg("o","prefix","Prefix to be used for snapshot files",false,"sim","filename");
    TCLAP::ValueArg<std::string> geneArg("g","gene-list","Gene list file",true,"null","filename");
    TCLAP::ValueArg<std::string> startArg("p","pop-desc","Population description file",false,"null","filename");
    TCLAP::ValueArg<std::string> libArg("l","gene-lib","Gene library directory",false,"files/genes/","filename");

    TCLAP::ValueArg<std::string> matrixArg("i","input","Input file defining the fitness landscape",false,"null","filename");
    
    // fitness function
    TCLAP::ValueArg<int> fitArg("f","fitness","Fitness function",false,1,"integer ID");
    
    // boolean switch to use DDG as input type
    TCLAP::ValueArg<std::string> inputArg("","sim-type","Define simulation type\n<s> (from selection coefficient, DMS or otherwise)\n<stability> (from DDG matrix or distribution)", false,"s","string");

    //use gamma distribution to draw selection coefficients
    TCLAP::SwitchArg gammaArg("","gamma","Draw selection coefficients from gamma distribution", cmd, false);

    //use normal distribution to draw selection coefficients
    TCLAP::SwitchArg normalArg("","normal","Draw selection coefficients from normal distribution", cmd, false);

    //first parameter of distribution
    TCLAP::ValueArg<double> alphaArg("","alpha","Alpha parameter of distribution\nGamma -> shape\nNormal -> mean",false,1,"double");

    //second parameter of distribution
    TCLAP::ValueArg<double> betaArg("","beta","Beta parameter of distribution\nGamma -> scale\nNormal -> S.D.",false,1,"double");

    // boolean switch to create population from scratch
    TCLAP::SwitchArg initArg("c","create-single","Create initial population on the fly", cmd, false);
    
    // boolean switch to enable analysis
    TCLAP::SwitchArg analysisArg("a","analysis","Enable analysis scripts", cmd, false);
    
    // boolean switch to track mutations
    TCLAP::SwitchArg eventsArg("e","track-events","Track mutation events", cmd, false);
    
    // boolean switch to use short format for snapshots
    TCLAP::SwitchArg shortArg("s","short-format","Use short format for population snapshots", cmd, false);

    // Add the arguments to the CmdLine object.
    cmd.add(maxArg);
    cmd.add(popArg);
    cmd.add(dtArg);
    cmd.add(prefixArg);
    cmd.add(geneArg);
    cmd.add(startArg);
    cmd.add(libArg);
    cmd.add(fitArg);
    cmd.add(matrixArg);
    cmd.add(alphaArg);
    cmd.add(betaArg);
    cmd.add(inputArg);

    // Parse the argv array.
    cmd.parse(argc, argv);

    // Get values from args. 
    GENERATION_MAX = maxArg.getValue();
    N = popArg.getValue();
    DT = dtArg.getValue();

    geneListFile = geneArg.getValue();
    outDir = prefixArg.getValue();
    startSnapFile = startArg.getValue();
    genesPath = libArg.getValue();

    inputType = inputArg.getValue();

    std::cout << "Begin ... " << std::endl;
    if(inputType == "s")
    {
        PolyCell::fromS_ = true;
        PolyCell::ff_ = 4;
        std::cout << "Initializing matrix ..." << std::endl;
        InitMatrix();
        std::cout << "Loading primordial genes file ..." << std::endl;
        LoadPrimordialGenes(geneListFile,genesPath);
        // if matrix is given
        if(matrixArg.isSet())
        {
            matrixFile = matrixArg.getValue();
            std::cout << "Extracting DMS matrix ..." << std::endl;
            ExtractDMSMatrix(matrixFile.c_str());
        }
        else
        {
            PolyCell::useDist_ = true;
            if(gammaArg.isSet())
            {
                Gene::shape_ = alphaArg.getValue();
                Gene::scale_ = betaArg.getValue();
                Gene::initGamma();
            }
            else if(normalArg.isSet())
            {
                Gene::mean_ = alphaArg.getValue();
                Gene::stdev_ = betaArg.getValue();
                Gene::initNormal();
            }
        }
    }
    else if(inputType == "stability")
    {
        std::cout << "Initializing matrix ..." << std::endl;
        InitMatrix();
        std::cout << "Loading primordial genes file ..." << std::endl;
        LoadPrimordialGenes(geneListFile,genesPath);
        PolyCell::ff_ = fitArg.getValue();
        // if DDG matrix is given
        if(matrixArg.isSet())
        {
            matrixFile = matrixArg.getValue();
            std::cout << "Extracting PDDG matrix ..." << std::endl;
            ExtractPDDGMatrix(matrixFile.c_str());
        }
        else
        {
            PolyCell::useDist_ = true;
        }
    }

    enableAnalysis = analysisArg.getValue();
    trackMutations = eventsArg.getValue();
    useShort = shortArg.getValue();
    createPop = initArg.getValue();

    }catch (TCLAP::ArgException &e){
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;}

    std::cout << "Opening starting population snapshot ..." << std::endl;
    std::fstream startsnap (startSnapFile.c_str(),std::ios::in|std::ios::binary);
    if (!startsnap.is_open()){
        std::cerr << "File could not be open: "<< startSnapFile << std::endl;
        exit(2);
    }
    
    // header
    int Total_Cell_Count;
    double frame_time;
    startsnap.read((char*)(&frame_time),sizeof(double));
    startsnap.read((char*)(&TIME),sizeof(double));
    startsnap.read((char*)(&Total_Cell_Count),sizeof(int));

    sprintf(buffer,"out/%s/snapshots",outDir.c_str());
    std::string outPath = buffer;
    std::cout << "Creating directory " << outPath << " ... " << (makePath(outPath) ? "OK" : "failed") << std::endl;

    std::vector <PolyCell> Cell_arr;
    double w_sum = 0;

    // IF POPULATION IS INITIALLY MONOCLONAL
    // CREATE VECTOR WITH N IDENTICAL CELLS
    // MINOR COMPUTATIONAL PENALTY DUE TO REATTRIBUTION OF BARCODES (BELOW)
    // BUT LARGELY OFFSET BY QUASI-INSTANTANEOUS INITIALIZATION OF VECTOR
    if(createPop){
        std::cout << "Creating a population of " << N << " cells ..." << std::endl;
        PolyCell A(startsnap, genesPath);
        Cell_arr.reserve(N);
        Cell_arr = std::vector <PolyCell>(N,A);
        for(std::vector<PolyCell>::iterator k = Cell_arr.begin(); k != Cell_arr.end(); ++k){
             (*k).ch_barcode(getBarcode());
        } 
    }
    else{
        // ELSE IT MUST BE POPULATED CELL BY CELL FROM SNAP FILE
        Cell_arr.reserve(N);
        int i=0;
        std::cout << "Constructing population from source " << startSnapFile.c_str() << " ..." << std::endl;
        while( i<Total_Cell_Count && !startsnap.eof()){
            PolyCell A(startsnap, genesPath);
            i++;
            Cell_arr.emplace_back(A);
        }
    }
    startsnap.close();
    std::cout << "Saving initial population snapshot ... " << std::endl;
    // save initial population snapshot
    sprintf(buffer,"%s/%s.gen%010d.snap",outPath.c_str(),outDir.c_str(), GENERATION_CTR); 

    // Open snapshot file
    std::fstream OUT2(buffer, std::ios::out | std::ios::binary);
    if (!OUT2.is_open()){
         std::cerr << "Snapshot file could not be opened";
         exit(1);
    }

    Total_Cell_Count = Cell_arr.size();
    OUT2.write((char*)(&frame_time),sizeof(double));
    OUT2.write((char*)(&TIME),sizeof(double));
    OUT2.write((char*)(&Total_Cell_Count),sizeof(int));

    if(useShort){
        for(std::vector<PolyCell>::iterator k = Cell_arr.begin(); k != Cell_arr.end(); ++k){
            w_sum += (*k).fitness();
            (*k).dumpShort(OUT2);
        } 
    }
    else{
        int l=1;
        // dump snapshot of initial population and get sum of fitnesses
        for(std::vector<PolyCell>::iterator k = Cell_arr.begin(); k != Cell_arr.end(); ++k){
            w_sum += (*k).fitness();
            (*k).dump(OUT2,l);
            l++;
        } 
    }
    
    OUT2.close();   

    std::ofstream MUTATIONLOG;
    if(trackMutations){
        // Open MUTATION LOG
        sprintf(buffer, "out/%s/MUTATION_LOG",outDir.c_str());
        MUTATIONLOG.open(buffer);
        if ( !MUTATIONLOG.is_open() ) {
            std::cerr << "Mutation log file could not be opened";
            exit(1);
        }
    }
    
    std::cout << "Starting evolution ..." << std::endl;

    // PSEUDO WRIGHT-FISHER PROCESS
    while(GENERATION_CTR < GENERATION_MAX){
        std::vector<PolyCell> Cell_temp;
        // reserve 2N to allow overflow and prevent segfault
        Cell_temp.reserve(N*2);
        // for each cell in the population

        for(std::vector<PolyCell>::iterator j = Cell_arr.begin(); j != Cell_arr.end(); ++j)
        {
            // fitness of cell j with respect to sum of population fitness
            double w = (*j).fitness()/w_sum;
            // probability parameter of binomial distribution
            std::binomial_distribution<> binCell(N, w);
            // number of progeny k is drawn from binomial distribution with N trials and mean w
            int k = binCell(rng);
            
            // if nil, the cell will be wiped from the population
            if(k == 0) continue;

            // iterator to current available position
            std::vector<PolyCell>::iterator it = end(Cell_temp);

            // iterator to end position of fill
            std::vector<PolyCell>::iterator last = it + k;

            // fill vector with k times the current cell
            std::fill_n(std::back_inserter(Cell_temp),k,(*j));

            // after filling with children, go through each one for mutation
            while(it < last){
                // potentially mutate
                if((*it).mrate()*(*it).genome_size() > RandomNumber())
                {
                    MUTATION_CTR++;
                    if(trackMutations){
                        // mutate and write mutation to file
                        (*it).ranmut_Gene(MUTATIONLOG,GENERATION_CTR);
                    }
                    else{
                        (*it).ranmut_Gene();
                    }       
                }
                std::advance(it,1);
            }
        }

        // if the population is below N
        // randomly draw from progeny to pad
        while(Cell_temp.size() < N){
            int s = Cell_temp.size();
            std::vector<PolyCell>::iterator j = Cell_temp.begin();
            Cell_temp.emplace_back((*(j+RandomNumber()*s)));
        }

        // if the population exceeds N
        // cells are randomly shuffled and the vector is shrunk to N
        if(Cell_temp.size() > N){
            auto engine = std::default_random_engine{};
            std::shuffle(Cell_temp.begin(), Cell_temp.end(), engine);
            Cell_temp.resize(N);
        }

        Total_Cell_Count = (int)(Cell_temp.size());
        assert(Total_Cell_Count == N);
        // swap population with initial vector
        Cell_arr.swap(Cell_temp);

        // reset and update w_sum
        // update Ns and Na for each cell
        w_sum = 0;
        for(std::vector<PolyCell>::iterator k = Cell_arr.begin(); k != Cell_arr.end(); ++k){
            w_sum += (*k).fitness();
            (*k).UpdateNsNa();
        }
        
        // update generation counter
        GENERATION_CTR++; 
        // save population snapshot every DT generations
        if( (GENERATION_CTR % DT) == 0){
             sprintf(buffer,"%s/%s.gen%010d.snap",outPath.c_str(),outDir.c_str(), GENERATION_CTR); 

             //Open snapshot file
             std::fstream OUT2(buffer, std::ios::out | std::ios::binary);
             if (!OUT2.is_open()){
                 std::cerr << "Snapshot file could not be opened";
                 exit(1);
             }
      
             double frame_time = GENERATION_CTR;
             OUT2.write((char*)(&frame_time),sizeof(double));
             OUT2.write((char*)(&TIME),sizeof(double));
             OUT2.write((char*)(&Total_Cell_Count),sizeof(int));

             if(useShort){
                    for(std::vector<PolyCell>::iterator k = Cell_arr.begin(); k != Cell_arr.end(); ++k){
                     (*k).dumpShort(OUT2);
                } 
             }
             else{
                int l=1;
                for(std::vector<PolyCell>::iterator k = Cell_arr.begin(); k != Cell_arr.end(); ++k){
                    (*k).dump(OUT2,l);
                    l++;
                }
             }
              
             OUT2.close();
         }
    }

    MUTATIONLOG.close();
    std::cout << "Done." << std::endl;
    std::cout << "Total number of mutation events: " << MUTATION_CTR << std::endl;
    // if the user toggled analysis, call shell script
    if(enableAnalysis){
        std::string script = "tools/barcodes.sh";
        std::string command = "/bin/bash "+script+" "+outDir+" "+std::to_string(GENERATION_MAX)+" "+std::to_string(N)+" "+std::to_string(DT)+" "+std::to_string((int) useShort);
        const char *cmd = command.c_str();
        system(cmd);
    }
    return 0;
}
