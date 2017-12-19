/*
 * particle_filter.cpp
 *
 *  Created on: Dec 12, 2016
 *      Author: Tiffany Huang
 */

#include <random>
#include <algorithm>
#include <iostream>
#include <numeric>
#include <math.h> 
#include <iostream>
#include <sstream>
#include <string>
#include <iterator>

#include "particle_filter.h"

using namespace std;

void ParticleFilter::init(double x, double y, double theta, double std[]) {
	// TODO: Set the number of particles. Initialize all particles to first position (based on estimates of 
	//   x, y, theta and their uncertainties from GPS) and all weights to 1. 
	// Add random Gaussian noise to each particle.
	// NOTE: Consult particle_filter.h for more information about this method (and others in this file).
	num_particles = 50;
	normal_distribution<double> dist_x(x, std[0]);
	normal_distribution<double> dist_y(y, std[1]);
	normal_distribution<double> dist_theta(theta, std[2]);
	default_random_engine gen;

	for (int i = 0; i < num_particles; i++) {
		Particle p;
		p.id = i;
		p.x = dist_x(gen);
		p.y = dist_y(gen);
		p.theta = dist_theta(gen);
		p.weight = 1;
		particles[i] = p;
		weights.push_back(1);
	}
	is_initialized = true;
}

void ParticleFilter::prediction(double delta_t, double std_pos[], double velocity, double yaw_rate) {
	// TODO: Add measurements to each particle and add random Gaussian noise.
	// NOTE: When adding noise you may find std::normal_distribution and std::default_random_engine useful.
	//  http://en.cppreference.com/w/cpp/numeric/random/normal_distribution
	//  http://www.cplusplus.com/reference/random/default_random_engine/
	default_random_engine gen;
	double v_over_yr = velocity / yaw_rate;
	double yr_dt = yaw_rate*delta_t;
	double theta;
	for (int i = 0; i < num_particles; i++) {
		theta = particles[i].theta;
		particles[i].x += v_over_yr * (sin(theta + yr_dt) - sin(theta));
		particles[i].y += v_over_yr * (cos(theta) - cos(theta + yr_dt));
		particles[i].theta += yr_dt;
		normal_distribution<double> dist_x(particles[i].x, std_pos[0]);
		normal_distribution<double> dist_y(particles[i].y, std_pos[1]);
		normal_distribution<double> dist_theta(particles[i].theta, std_pos[2]);
		particles[i].x = dist_x(gen);
		particles[i].y = dist_y(gen);
		particles[i].theta = dist_theta(gen);
	}
}

void ParticleFilter::dataAssociation(std::vector<LandmarkObs> predicted, std::vector<LandmarkObs>& observations) {
	// TODO: Find the predicted measurement that is closest to each observed measurement and assign the 
	//   observed measurement to this particular landmark.
	// NOTE: this method will NOT be called by the grading code. But you will probably find it useful to 
	//   implement this method and use it as a helper during the updateWeights phase.
	for (int i = 0; i < observations.size(); i++) {
		double dist_min = numeric_limits<double>::max();
		for (int j = 0; j < predicted.size(); j++) {
			if (dist(observations[i].x, observations[i].y, predicted[j].x, predicted[j].y) < dist_min)
			{
				observations[i].id = predicted[j].id;
			}
		}
	}
}

void ParticleFilter::updateWeights(double sensor_range, double std_landmark[], 
		const std::vector<LandmarkObs> &observations, const Map &map_landmarks) {
	// TODO: Update the weights of each particle using a multi-variate Gaussian distribution. You can read
	//   more about this distribution here: https://en.wikipedia.org/wiki/Multivariate_normal_distribution
	// NOTE: The observations are given in the VEHICLE'S coordinate system. Your particles are located
	//   according to the MAP'S coordinate system. You will need to transform between the two systems.
	//   Keep in mind that this transformation requires both rotation AND translation (but no scaling).
	//   The following is a good resource for the theory:
	//   https://www.willamette.edu/~gorr/classes/GeneralGraphics/Transforms/transforms2d.htm
	//   and the following is a good resource for the actual equation to implement (look at equation 
	//   3.33
	//   http://planning.cs.uiuc.edu/node99.html

	//For each particle
	double xp, yp, thetap, xc, yc;
	vector<double> ws;
	for (int i = 0; i < num_particles; i++) {
		xp = particles[i].x;
		yp = particles[i].y;
		thetap = particles[i].theta;

		//Transform noisy observations made in car coordinates into map coordinates
		vector<LandmarkObs> obs_map;
		for (int j = 0; j < observations.size(); j++) {
			xc = observations[j].x;
			yc = observations[j].y;
    		LandmarkObs obs;
    		obs.x = xp + cos(thetap)*xc - sin(thetap)*yc;
			obs.y = yp + sin(thetap)*xc + cos(thetap)*yc;
			obs_map.push_back(obs);
		}

		//Associate observations with predicted landmarks
		vector<LandmarkObs> predicted;
		double xl, yl;
		for (int j = 0; j < map_landmarks.landmark_list.size(); j++) {
			xl = map_landmarks.landmark_list[j].x_f;
			yl = map_landmarks.landmark_list[j].y_f;
			if (dist(xp, yp, xl, yl) <= sensor_range) {  //Discard landmarks assumed to be out of range for the particle
	    		LandmarkObs obs;
	    		obs.id = map_landmarks.landmark_list[j].id_i;
	    		obs.x = xl - xp;
	    		obs.y = yl - yp;
	    		predicted.push_back(obs);
			}
		}
		dataAssociation(predicted, obs_map);

		//Update weights based on distance between observations and landmarks
		double w = 1;
		double c1 = 1 / (2*M_PI*std_landmark[0]*std_landmark[1]);
		double c2 = 2*std_landmark[0]*std_landmark[0];
		double c3 = 2*std_landmark[1]*std_landmark[1];
		for (int j = 0; j < obs_map.size(); j++) {
			xc = obs_map[j].x;
			yc = obs_map[j].y;
			xl = 0, yl = 0;
			for (int k = 0; map_landmarks.landmark_list.size(); k++) {
				if (obs_map[j].id == map_landmarks.landmark_list[k].id_i) {
					xl = map_landmarks.landmark_list[k].x_f;
					yl = map_landmarks.landmark_list[k].y_f;
					break;
				}
			}
			w *= c1 * exp(-((pow(xc - xl, 2)/c2) + (pow(yc - yl, 2)/c3)));
		}
		particles[i].weight = w;
		ws.push_back(w);
	}
	weights = ws;
}

void ParticleFilter::resample() {
	// TODO: Resample particles with replacement with probability proportional to their weight. 
	// NOTE: You may find std::discrete_distribution helpful here.
	//   http://en.cppreference.com/w/cpp/numeric/random/discrete_distribution
	discrete_distribution<> d(weights.begin(), weights.end());
	default_random_engine gen;
	vector<Particle> newParticles;
	for (int i = 0; i < num_particles; i++) {
		Particle sampledParticle = particles[d(gen)];
		Particle p;
		p.id = i;
		p.x = sampledParticle.x;
		p.y = sampledParticle.y;
		p.theta = sampledParticle.theta;
		p.weight = 1;
		newParticles.push_back(p);
	}
	particles = newParticles;
}

Particle ParticleFilter::SetAssociations(Particle& particle, const std::vector<int>& associations, 
                                     const std::vector<double>& sense_x, const std::vector<double>& sense_y)
{
    //particle: the particle to assign each listed association, and association's (x,y) world coordinates mapping to
    // associations: The landmark id that goes along with each listed association
    // sense_x: the associations x mapping already converted to world coordinates
    // sense_y: the associations y mapping already converted to world coordinates

    particle.associations= associations;
    particle.sense_x = sense_x;
    particle.sense_y = sense_y;
}

string ParticleFilter::getAssociations(Particle best)
{
	vector<int> v = best.associations;
	stringstream ss;
    copy( v.begin(), v.end(), ostream_iterator<int>(ss, " "));
    string s = ss.str();
    s = s.substr(0, s.length()-1);  // get rid of the trailing space
    return s;
}
string ParticleFilter::getSenseX(Particle best)
{
	vector<double> v = best.sense_x;
	stringstream ss;
    copy( v.begin(), v.end(), ostream_iterator<float>(ss, " "));
    string s = ss.str();
    s = s.substr(0, s.length()-1);  // get rid of the trailing space
    return s;
}
string ParticleFilter::getSenseY(Particle best)
{
	vector<double> v = best.sense_y;
	stringstream ss;
    copy( v.begin(), v.end(), ostream_iterator<float>(ss, " "));
    string s = ss.str();
    s = s.substr(0, s.length()-1);  // get rid of the trailing space
    return s;
}
