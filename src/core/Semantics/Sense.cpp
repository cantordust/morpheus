#include "Sense.hpp"


namespace Morpheus
{
    Sense::Sense()
    {

    }

    void Sense::test()
    {
	input_layer = Eigen::VectorXd::Random(300000);

	hidden_layer = Eigen::VectorXd::Random(Config::hidden_layer_size);
	recurrent_layer.resize(Config::hidden_layer_size);
	output_layer.resize(300000);

	input_hidden = Eigen::MatrixXd::Random(hidden_layer.size(),input_layer.size());
	hidden_output = Eigen::MatrixXd::Random(output_layer.size(),hidden_layer.size());

	/// Input-hidden matrix * input layer
	hidden_layer = input_hidden * input_layer;

	recurrent_layer = hidden_layer;
    }
}
