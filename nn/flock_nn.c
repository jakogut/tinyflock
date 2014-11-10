#include <fann.h>

int main()
{
	unsigned int num_input = 512,
		     num_output = 512;

	float desired_error = 0.05;

	unsigned max_epochs = 1024,
		 epochs_between_reports = 25;

	struct fann *ann = fann_create_standard(6, num_input, 512, 512, 512, 512, num_output);

	fann_set_activation_function_hidden(ann, FANN_SIGMOID_SYMMETRIC);
	fann_set_activation_function_output(ann, FANN_SIGMOID_SYMMETRIC);

	fann_train_on_file(ann, "flock.dat", max_epochs, epochs_between_reports, desired_error);

	fann_save(ann, "flock.net");

	fann_destroy(ann);

	return 0;
}
