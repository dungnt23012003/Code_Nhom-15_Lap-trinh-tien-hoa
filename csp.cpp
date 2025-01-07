#include<iostream>
#include<fstream>
#include<vector>
#include<random>
#include<algorithm>
#include<map>
#include<set>
#include <chrono>

using namespace std;

int found_at;
double best;
random_device rd;
mt19937 random_engine(rd());

/****************************************************************/
// Định nghĩa các đối tượng bài toán
/****************************************************************/
class Problem{
public:
    int num_stock;
    vector<int> stock_length;
    int num_item;
    int num_order;
    vector<int> item_length;
    vector<int> num_item_each_length;

    static Problem from_file(const string& file_path);

};

Problem Problem::from_file(const string& file_path){
    ifstream file(file_path);
    if(!file.is_open()){
        throw runtime_error("can not open file");
    }

    int tmp, max_element=0;

    Problem problem;
    file >> problem.num_stock;

    for(int i=0;i<problem.num_stock;i++){
        file >> tmp;
        max_element = max(tmp, max_element);
        problem.stock_length.push_back(tmp);
    }

    problem.num_item_each_length.resize(max_element);

    int num_item=0;
    file >> problem.num_order;
    for(int i=0;i<problem.num_order;i++){
        file >> tmp;
        problem.item_length.push_back(tmp);
        file >> tmp;
        num_item = num_item + tmp;
        problem.num_item_each_length[problem.item_length[i]] = tmp;
    }
    problem.num_item = num_item;

    return problem;
}

class Individual{
public:
    vector<vector<int>> chromosome;
    double fitness;

    explicit Individual(vector<vector<int>> chromosome, double fitness): chromosome(move(chromosome)), fitness(fitness) {}; 
};

pair<int, int> wastage(const Problem& problem, const vector<int>& gene){
    int wastage = 0;

    for(int x: gene){
        wastage += x;
    }

    for(int i: problem.stock_length){
        if(i>=wastage){
            wastage = i - wastage;
            return make_pair(wastage, i);
        }
    }
    
    return make_pair(-1, -1);
}

double fitness_without_contiguity(const Problem& problem, const vector<vector<int>>& chromosome){
    double fitness = 0;
    int num_waste = 0;
    pair<int, int> wastage_group;

    for(const vector<int>& gene:chromosome){
        wastage_group = wastage(problem, gene);
        if(wastage_group.first>0){
            num_waste++;
        }
        fitness = fitness + sqrt(1.0*wastage_group.first/wastage_group.second);
        if(wastage_group.first==-1){
            throw runtime_error("chromosome is not valid");
        }
    }
    return 1-(fitness+ 1.0*num_waste/chromosome.size())/(chromosome.size()+1);

}

double fitness_with_contiguity(const Problem& problem, const vector<vector<int>>& chromosome){
    double fitness = 0;
    double tmp = 0;
    vector<int> item_need_to_cut = problem.num_item_each_length;
    int num_open=0;
    pair<int, int> wastage_group;
    set<int> item_current_cut;
    for(const vector<int>& gene:chromosome){
        wastage_group = wastage(problem, gene);
        for(const int& i:gene){
            item_current_cut.insert(i);
            item_need_to_cut[i]--;
            if(item_need_to_cut[i]==0){
                item_current_cut.erase(i);
            }
        }
        num_open = item_current_cut.size();
        tmp = tmp+ (1.0*num_open/problem.num_order)*(1.0*num_open/problem.num_order);
        fitness = fitness + sqrt(1.0*wastage_group.first/wastage_group.second);
        if(wastage_group.first==-1){
            throw runtime_error("chromosome is not valid");
        }
    }
    return 1-(fitness+10*tmp/chromosome.size())/(chromosome.size()+10);

}


bool operator<(const Individual& first, const Individual& second){
    return first.fitness > second.fitness; 
};


vector<vector<int>> grouping(const Problem& problem, vector<int> a){
    bool is_cut[a.size()];
    for(int i=0;i<a.size();i++){
        is_cut[i]=false;
    }

    uniform_int_distribution<> distribution(0, problem.num_stock-1);

    vector<vector<int>> chromosome;
    vector<int> gene_tmp;
    int stock_length_tmp;
    int num_item_cut = 0;

    while(num_item_cut<a.size()){
        stock_length_tmp=problem.stock_length[distribution(random_engine)];
        for(int i=0;i<a.size();i++){
            if(is_cut[i] || stock_length_tmp < a[i]){
                continue;
            }
            
            gene_tmp.push_back(a[i]);
            num_item_cut++;
            is_cut[i]=true;
            stock_length_tmp -= a[i];
        }
        chromosome.push_back(move(gene_tmp));
    }

    return chromosome;
    
}

vector<vector<int>> mutation(const Problem& problem, const vector<vector<int>>& chromosome_parent, int generation, int max_generation){
    
    vector<int> check;
    check.resize(problem.num_item_each_length.size());
    for(int i=0;i<check.size();i++){
        check[i]=0;
    }
    for(int i=0;i<chromosome_parent.size();i++){
        for(int j=0;j<chromosome_parent[i].size();j++){
            check[chromosome_parent[i][j]]++;
        }
    }
    int scaling_factor = 100;
    
    vector<double> all_wastage;
    for(int i=0;i<chromosome_parent.size();i++){
        pair<int, int> wastage_tmp = wastage(problem, chromosome_parent[i]);
        all_wastage.push_back(wastage_tmp.first);
        
    }

    vector<int> probability;
    for(int i=0;i<chromosome_parent.size();i++){
        probability.push_back((int)(1.0-1.0*generation/max_generation)*scaling_factor*all_wastage[i] + 1);
    }



    discrete_distribution<> distribution(probability.begin(), probability.end());
    int first_random_index = distribution(random_engine);
    probability[first_random_index]=0;
    
    discrete_distribution<> new_distribution(probability.begin(), probability.end());
    int second_random_index = new_distribution(random_engine);

    while(first_random_index==second_random_index){
        cout << generation << " \n";
    }

    if(first_random_index>second_random_index){
        swap(first_random_index, second_random_index);
    }
    

    vector<vector<int>> chromosome;

    vector<int> item_to_grouping_1;
    vector<int> item_to_grouping_2;

    for(int i=0;i<chromosome_parent[first_random_index].size();i++){
        item_to_grouping_1.push_back(chromosome_parent[first_random_index][i]);
    }

    for(int i=0;i<chromosome_parent[second_random_index].size();i++){
        item_to_grouping_2.push_back(chromosome_parent[second_random_index][i]);
    }

    uniform_int_distribution<> distribution1(0, item_to_grouping_1.size()-1);
    uniform_int_distribution<> distribution2(0, item_to_grouping_2.size()-1);

    int first, second;
    first = distribution1(random_engine);
    second = distribution2(random_engine);

    swap(item_to_grouping_1[first], item_to_grouping_2[second]);

    for(int i=0;i<item_to_grouping_2.size();i++){
        item_to_grouping_1.push_back(item_to_grouping_2[i]);
    }

    for(int i=0;i<chromosome_parent.size();i++){
        if(i!=first_random_index && i!=second_random_index){
            chromosome.push_back(chromosome_parent[i]);
        }
    }

    vector<vector<int>> chromosome_grouping = grouping(problem, item_to_grouping_1);
    for (int i = 0; i < chromosome_grouping.size(); i++) {
        chromosome.push_back(move(chromosome_grouping[i]));
    }
    for(int i=0;i<chromosome.size();i++){
        for(int j=0;j<chromosome[i].size();j++){
            check[chromosome[i][j]]--;
        }
    }
    
    for(int k=0;k<check.size();k++){
        if(check[k]){
            for(int i=0;i<chromosome_parent.size();i++){
                for(int j=0;j<chromosome_parent[i].size();j++){
                    cout << chromosome_parent[i][j] << " ";
                }
                cout << "\n";
            }
            cout << "\n";
            for(int i=0;i<chromosome.size();i++){
                for(int j=0;j<chromosome[i].size();j++){
                    cout << chromosome[i][j] << " ";
                }
                cout << "\n";
            }
            cout << "\n";
            cout << "index swap " << first_random_index << " " << second_random_index << " " << k << " " <<  first << " " << second<< "\n";
        }
    }

    return chromosome;
}



class EvolutionalProgramingOption{
public:
    explicit EvolutionalProgramingOption(const Problem& problem): problem(problem) {}

    const Problem problem;
    int population_size = 100;
    int max_generation = 1000;
    double elitist_percentage = 0.3;
    bool with_contiguity = false;
    vector<vector<int>> (*mutation)(const Problem& problem, const vector<vector<int>>& chromosome, int generation, int max_generation) = mutation;
    double (*fitness)(const Problem& problem, const vector<vector<int>>& chromosome) = fitness_without_contiguity;

    int elitist_count() const {
        return (int) (elitist_percentage * population_size);
    }

};



vector<vector<int>> create_random_chromosome(const EvolutionalProgramingOption& option){
    vector<int> chromosome_tmp;
    for(int item: option.problem.item_length){
        for(int j=0;j<option.problem.num_item_each_length[item];j++){
            chromosome_tmp.push_back(item);
        }
    }

    shuffle(chromosome_tmp.begin(), chromosome_tmp.end(), random_engine);
    
    return grouping(option.problem, chromosome_tmp);
}

vector<Individual> create_random_population(const EvolutionalProgramingOption& option, int population_size){
    vector<Individual> population;
    population.reserve(population_size);

    for(int i=0;i<population_size;i++){
        vector<vector<int>> chromosome = create_random_chromosome(option);
        population.emplace_back(
            chromosome, option.fitness(option.problem, chromosome));
    }
    return population;
}

vector<Individual> create_offsprings(const vector<Individual>& population, const EvolutionalProgramingOption& option, int generation){
    vector<Individual> offsprings;
    offsprings.reserve(population.size());
    for(int i=0;i<population.size();i++){
        vector<vector<int>> chromosome = option.mutation(option.problem, population[i].chromosome, generation, option.max_generation);
        offsprings.emplace_back(chromosome, option.fitness(option.problem, chromosome));
    }

    return offsprings;
}



void next_generation(vector<Individual>& population, const EvolutionalProgramingOption& option, int generation){

    sort(population.begin(), population.end());
    int elitist_count = option.elitist_count();

    vector<Individual> offsprings = create_offsprings(population, option, generation);
    sort(offsprings.begin(), offsprings.end());


    for(int i=elitist_count;i<population.size();i++){
        population[i]=offsprings[i-elitist_count];
    }
}

void print_solution(const Individual& individual, const EvolutionalProgramingOption& option){
    cout << "\n";
    int total_wastage=0;
    vector<int> item_need_to_cut = option.problem.num_item_each_length;
    int num_open=0;
    set<int> item_current_cut;
    for(int i=0;i<individual.chromosome.size();i++){
        for(const int& item: individual.chromosome[i]){
            item_current_cut.insert(item);
            item_need_to_cut[item]--;
            if(item_need_to_cut[item]==0){
                item_current_cut.erase(item);
            }
        }
        num_open = item_current_cut.size();
        cout << i << ": ";
        for(int j=0;j<individual.chromosome[i].size();j++){
            cout << individual.chromosome[i][j] << " ";
        }
        pair<int, int> waste = wastage(option.problem, individual.chromosome[i]);
        total_wastage = total_wastage + waste.first;
        cout << "| waste: " << waste.first << "/" << waste.second;
        if(option.with_contiguity){
            cout << " | open: " << num_open << "\n"; 
        }
        else{
            cout << "\n";
        }
    }
    cout << "fitness: " << individual.fitness << " total_wastage: " << total_wastage << " ";
}
Individual EP_solve_CSP(const EvolutionalProgramingOption& option){
    vector<Individual> population = create_random_population(option, option.population_size);

    int generation =1;
    // cout << "population init" << "\n";
    // print_solution(*min_element(population.begin(), population.end()), option);

    while(generation<option.max_generation){
        next_generation(population, option, generation);
        Individual min_tmp = *min_element(population.begin(), population.end());
        // if(generation%50==0){
        //     cout << "population " << generation << "\n";
        //    print_solution(min_tmp, option);
        // } 
        if(min_tmp.fitness>best){
            best = min_tmp.fitness;
            found_at=generation;
        }
        generation++;
    }

    return *min_element(population.begin(), population.end());
}
int main(){


    freopen("result/40/output5a.txt", "w", stdout);
    Problem problem = Problem::from_file("dataset_csp/problem5a.csp");

    EvolutionalProgramingOption option(problem);
    
    int num_run = 20;
    option.population_size = 40;
    option.max_generation = 1000;
    option.elitist_percentage = 0.5;

    option.mutation = mutation;
    option.with_contiguity = false;
    option.fitness = fitness_without_contiguity;
    
    
    double result_tmp[20];
    int found_at_tmp[20]; 
    for(int i=0;i<num_run;i++){
        found_at = -1;
        best = -1;
        Individual result = EP_solve_CSP(option);
        cout << "\nresult " << i+1;
        print_solution(result, option);
        cout << "found at: " << found_at << "\n";
        result_tmp[i]=result.fitness;
        found_at_tmp[i]=found_at;
    }
    cout << "\n\n";
    for(int i=0;i<num_run;i++){
        cout << result_tmp[i] <<  ", "[i==num_run-1] << " \n"[i==num_run-1];
    }
    for(int i=0;i<num_run;i++){
        cout << found_at_tmp[i] << ", "[i==num_run-1] << " \n"[i==num_run-1];
    }
    return 0;
}