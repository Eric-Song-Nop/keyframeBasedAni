#ifndef CUBICSPLINER
#define CUBICSPLINER

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <eigen3/Eigen/Eigen>
#include <iostream>

namespace cubic{

    class Spliner{
        public:
        Spliner(){N = 0;}
        Spliner(int, std::vector<float>, std::vector<float>);
        float getY(float);
        void setXY(int, std::vector<float>, std::vector<float>);
        void clear(){
            x.clear();
            y.clear();
            N = 0;
        }

        private:
        std::vector<float> x;
        std::vector<float> y;
        Eigen::VectorXf M;
        int N;
        
        float getStep(int);
        void calcuMVector();
        float get_a(int);
        float get_b(int);
        float get_c(int);
        float get_d(int);
        int get_phase(float);
    };

    
    Spliner::Spliner(int n, std::vector<float> x, std::vector<float> y){
        N = n + 2;
        this->x = x;
        this->y = y;
        this->x.insert(this->x.begin(), 0.0f);
        this->y.insert(this->y.begin(), 0.0f);
        this->x.push_back(10.0f);
        this->y.push_back(0.0f);
        calcuMVector();
    }

    void Spliner::setXY(int n, std::vector<float> x, std::vector<float> y){
        N = n + 2;
        this->x = x;
        this->y = y;

        this->x.insert(this->x.begin(), 0.0f);
        this->y.insert(this->y.begin(), 0.0f);
        this->x.push_back(10.0f);
        this->y.push_back(0.0f);

        calcuMVector();
    }

    float Spliner::getStep(int n){
        return x[n+1] - x[n];
    }

    void Spliner::calcuMVector(){
        Eigen::MatrixXf matA(N, N); 
        Eigen::VectorXf matB(N);
        // Eigen::VectorXf M_vec(N);
        matB(0) = 0;
        matB(N-1) = 0;
        // matA(0,0) = getStep(0) * 2;
        // matA(0,1) = getStep(0);
        matA(0,0)= 1;
        // matA(N-1, N-1) = 2 * getStep(N-2);
        // matA(N-1, N-2) = getStep(N-2);
        matA(N-1,N-1)=1;
        for(int i = 1; i != N-1; i++){
            matB(i) = 6*(y[i+1] - y[i])/getStep(i) - 6 *(y[i] - y[i-1])/getStep(i-1);
            matA(i, i-1) = getStep(i-1);
            matA(i, i+1) = getStep(i);
            matA(i, i) = 2*(getStep(i-1) + getStep(i));
        }
        this->M = matA.inverse() * matB;
    }

    float Spliner::get_a(int n){
        return y[n];
    }

    float Spliner::get_b(int n){
        return (y[n+1] - y[n]) / getStep(n) - getStep(n)*M(n)/2 - getStep(n)*(M(n+1)-M(n))/6;
    }

    float Spliner::get_c(int n){
        return M(n) / 2;
    }

    float Spliner::get_d(int n){
        return (M(n+1) - M(n))/(6*getStep(n));
    }

    int Spliner::get_phase(float x){
        for(int i = 0; i != N-1; i++){
            if(this->x[i]<= x && this->x[i+1] >x){
                return i;
            }
        }
    }

    float Spliner::getY(float x){
        int n = get_phase(x);
        auto x_t = x - this->x[n];
        return (get_a(n) + x_t * get_b(n) + x_t*x_t*get_c(n) + x_t*x_t*x_t*get_d(n));
    }
}

#endif