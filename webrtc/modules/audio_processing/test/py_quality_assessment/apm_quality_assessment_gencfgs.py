"""Generate .json files.

 Files with which the APM module can be tested using the
   apm_quality_assessment.py script.
"""

import itertools
import logging
import os

import quality_assessment.data_access as data_access

OUTPUT_PATH = os.path.abspath("apm_configs")


def _GenerateCombinations():
  """Generates a big list configs with different parameters.

  All the configurations are written to files in OUTPUT_PATH.
  """

  config_params = {
      "-agc2_fd_le_a": [0, 0.1, 0.2],  # attack
      "-agc2_fd_le_d":[0, 1, 10, 100],  # decay
      "-agc2_fd_le_n":[0, 5, 10, 20],  # sub frames
      "-agc2_fd_g":  [-5, 0, 5],  # gain
  }

  param_names = [
      "-agc2_fd_le_a", "-agc2_fd_le_d", "-agc2_fd_le_n", "-agc2_fd_g"
  ]
  other_names = ["attack", "decay", "frames", "gain"]
  param_combs = [config_params[key] for key in param_names]

  for param_comb in itertools.product(*param_combs):
    config = {x: y for (x, y) in zip(param_names, param_comb)}

    name = "__".join([
        "{}-{}".format(name, value)
        for (name, value) in zip(other_names, param_comb)
    ])

    config_filepath = os.path.join(OUTPUT_PATH, name + ".json")
    logging.debug("config file <%s> | %s", config_filepath, config)

    data_access.AudioProcConfigFile.Save(config_filepath, config)
    logging.info("config file created: <%s>", config_filepath)


def main():
  logging.basicConfig(level=logging.INFO)
  _GenerateCombinations()


if __name__ == "__main__":
  main()
