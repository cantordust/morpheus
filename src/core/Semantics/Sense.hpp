#ifndef SENSE_HPP
#define SENSE_HPP

#include "Config.hpp"
#include "Globals.hpp"
#include <eigen3/Eigen/Dense>

namespace Morpheus
{
    class Sense : public QObject
    {
        Q_OBJECT
    public:
        Sense();

        /// Network layers
        Eigen::VectorXd input_layer;
        Eigen::VectorXd hidden_layer;
        Eigen::VectorXd recurrent_layer;
        Eigen::VectorXd output_layer;

        /// Connections
        Eigen::MatrixXd input_hidden;
        Eigen::MatrixXd hidden_output;

    public slots:

        void test();

    };
}

#endif // SENSE_HPP
