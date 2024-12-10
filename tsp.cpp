#include<iostream>
#include<fstream>
#include<vector>
#include<random>
#include<algorithm>

using namespace std;

random_device rd;
mt19937 random_engine(rd());

/****************************************************************/
// Định nghĩa các đối tượng bài toán
/****************************************************************/
class Problem{
    private:
        int n;
        double* distances;
        static double euclid_2d(double x1, double y1, double x2, double y2){
            return sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));
        }
    public:
        explicit Problem(int n): n(n), distances(new double[n*n]){};

        ~Problem(){
            delete[] distances;
        }

        int size() const{
            return n;
        }

        double& distance(int i, int j){
            return distances[i*n + j];
        }

        const double& distance(int i, int j) const{
            return distances[i*n+j];
        }

        void test(){
            for(int i=0;i<n;i++){
                for(int j=0;j<n;j++){
                    cout << distances[i*n+j] << " \n"[j==n-1];
                }
            }
        }

        static Problem from_tsplib(const string& file_path);

};
// Đọc bài toán từ file
Problem Problem::from_tsplib(const string& file_path){
    ifstream file(file_path);
    string tmp;

    if (!file.is_open()) {
        std::runtime_error("Can't open file");
    }

    getline(file, tmp);
    getline(file, tmp);
    getline(file, tmp);

    while (file.peek() != ':') file.ignore();
    file.ignore();

    int n;
    file >> n;
    Problem problem(n);
    file.ignore();

    getline(file, tmp);
    getline(file, tmp);

    double x[n], y[n];
    for(int i=0;i<n;i++){
        file >> x[i] >> x[i] >> y[i];
    }
    for(int i=0;i<n;i++){
        for(int j=0;j<n;j++){
            problem.distance(i,j) = Problem::euclid_2d(x[i], y[i], x[j], y[j]);
        }
    }

    return problem;
}

/****************************************************************/
// Định nghĩa các đối tượng di truyền
/****************************************************************/

using Gene = vector<int>;

class Individual{
public:
    Gene gene;
    double fitness;


    Individual(Gene gene, double fitness): gene(move(gene)), fitness(fitness) {}
    
};      
// Tính fitness của một gene
double fitness(const Problem& problem, const Gene& gene){
    double result = 0;
    for(int i=0;i<gene.size()-1;i++){
        result = result + problem.distance(gene[i], gene[i+1]);
    }
    result = result + problem.distance(gene[gene.size()-1], gene[0]);
    return result;
}
// Định nghĩa toán tử so sánh bé hơn cho các Individual dựa trên fitness
bool operator<(const Individual& first, const Individual& second){
    return first.fitness < second.fitness; 
};
// Định nghĩa toán tử in Individual
ostream& operator<<(ostream& os, const Individual& individual){
    if(!individual.gene.size()){
        os << "fitness: 0, []";
        return os;
    }

    os << "fitness: " << individual.fitness << ", [" << individual.gene[0];
    for(int i=1;i<individual.gene.size();i++){
        os << ", "  << individual.gene[i];
    }
    os << "]\n";
    return os;
}
// Tạo một gene ngẫu nhiên
Gene create_random_gene(const Problem& problem){
    Gene gene(problem.size());
    for(int i=0;i<gene.size();i++){
        gene[i]=i;
    }
    shuffle(gene.begin(), gene.end(), random_engine);

    return gene;
}
// Tạo một quần thể ngẫu nhiên
vector<Individual> create_random_population(const Problem& problem, int population_size){
    vector<Individual> population;
    population.reserve(population_size);

    for(int i=0;i<population_size;i++){
        Gene&& gene = create_random_gene(problem);
        population.emplace_back(gene, fitness(problem, gene));
    }
    return population;
}
// Đột biến
Gene mutation(const Gene& parent_gene, int generation, int max_generation){
    Gene gene = parent_gene;

    uniform_int_distribution<> distribution1(0, gene.size()-1);
    int index1= distribution1(random_engine);
    int dis = gene.size() - gene.size()*generation/max_generation;
    // dis=max((int)gene.size()/100,dis);
    uniform_int_distribution<> distribution2(max(0, index1-dis), min((int)gene.size()-1, index1+dis));
    int index2= distribution2(random_engine);
    while(index1==index2){
        index2=distribution2(random_engine);
    }
    if(index1>index2){
        swap(index1, index2);
    }
    reverse(gene.begin()+index1, gene.begin() + index2 + 1);

    return gene;
}
// EvolutionalProgramingOption

class EvolutionalProgramingOption{
public:
    explicit EvolutionalProgramingOption(const Problem& problem): problem(problem) {}

    const Problem problem;
    int population_size = 100;
    int max_generation = 1000;
    double elitist_percentage = 0.3;

    Gene (*mutation)(const Gene& gene, int generation, int max_generation) = mutation;

    int elitist_count() const {
        return (int) (elitist_percentage * population_size);
    }

};

vector<Individual> create_offsprings(vector<Individual>& population, const EvolutionalProgramingOption& option, const int generation){
    vector<Individual> offsprings;
    offsprings.reserve(population.size());
    for(int i=0;i<population.size();i++){
        Gene gene = mutation(population[i].gene, generation, option.max_generation);
        offsprings.emplace_back(gene, fitness(option.problem, gene));
    }

    return offsprings;
}
void next_generation(vector<Individual>& population, const EvolutionalProgramingOption& option, const int generation){

    sort(population.begin(), population.end());
    int elitist_count = option.elitist_count();

    vector<Individual> offsprings = create_offsprings(population, option, generation);
    sort(offsprings.begin(), offsprings.end());


    for(int i=elitist_count;i<population.size();i++){
        population[i]=offsprings[i-elitist_count];
    }
}

Individual EP_solve_TSP(const EvolutionalProgramingOption& option){
    vector<Individual> population = create_random_population(option.problem, option.population_size);

    int generation =1;
    cout << "population init" << "\n";
    cout << *min_element(population.begin(), population.end());

    while(generation<option.max_generation){
        next_generation(population, option, generation);

        if(generation%500==0){
            cout << "population " << generation << "\n";
            cout << *min_element(population.begin(), population.end());
        }
        generation++;
    }

    return *min_element(population.begin(), population.end());
}

int main(){
    // ios_base::sync_with_stdio(false);
    // cin.tie(NULL);

    Problem problem = Problem::from_tsplib("D:/c++/EvolutionalComputation/rat195.tsp");
    EvolutionalProgramingOption option(problem);

    option.population_size = 100;
    option.max_generation = 5000;
    option.elitist_percentage = 0.3;
    option.mutation = mutation;
    
    Individual result = EP_solve_TSP(option);
    cout << "result\n";
    cout << result;

    return 0;
}