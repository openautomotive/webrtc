#!/usr/bin/env python
# Copyright (c) 2017 The WebRTC project authors. All Rights Reserved.
#
# Use of this source code is governed by a BSD-style license
# that can be found in the LICENSE file in the root of the source
# tree. An additional intellectual property rights grant can be found
# in the file PATENTS.  All contributing project authors may
# be found in the AUTHORS file in the root of the source tree.

import argparse
import logging
import os
import re

import quality_assessment.data_access as data_access
import apm_quality_assessment_collect_data as collect_data

def _InstanceArgumentsParser():
  """Arguments parser factory.
  """
  parser = argparse.ArgumentParser(description=(
      'Rudimentary optimization of a function over different parameter'
      'combinations.'))

  parser.add_argument('-o', '--output_dir', required=True,
                      help=('the same base path used with the '
                            'apm_quality_assessment tool'))

  parser.add_argument('-c', '--config_names', type=re.compile,
                      help=('regular expression to filter the APM configuration'
                            ' names'))

  parser.add_argument('-i', '--capture_names', type=re.compile,
                      help=('regular expression to filter the capture signal '
                            'names'))

  parser.add_argument('-r', '--render_names', type=re.compile,
                      help=('regular expression to filter the render signal '
                            'names'))

  parser.add_argument('-e', '--echo_simulator_names', type=re.compile,
                      help=('regular expression to filter the echo simulator '
                            'names'))

  parser.add_argument('-t', '--test_data_generators', type=re.compile,
                      help=('regular expression to filter the test data '
                            'generator names'))

  parser.add_argument('-s', '--eval_scores', type=re.compile,
                      help=('regular expression to filter the evaluation score '
                            'names'))

  parser.add_argument('-n', '--config_dir', required=False,
                      help=('path to the folder with the configuration files'),
                      default='apm_configs')

  parser.add_argument('-p', '--params', required=True, nargs='+',
                      help=('parameters to parse from the config files in'
                            'config_dir'))

  return parser


def _ConfigurationAndScores(data_frame, params, config_dir):
  """Returns a list of all configurations and scores.

  Every result is of the form {params: {key/values}, scores :
  {key/values}}.

  Args:
    params: The parameter names to parse from configs the config
            directory
    data_frame: A pandas data frame with the scores and config name
                returned by _FindScores.
    config_dir: Path to folder with config files.
  """
  results = []
  config_names = data_frame['apm_config'].drop_duplicates().values.tolist()
  score_names = data_frame.eval_score_name.unique().tolist()

  for config_name in config_names:
    config_json = data_access.AudioProcConfigFile.Load(
        os.path.join(config_dir, config_name + ".json"))
    scores = {}
    data_cell = data_frame[data_frame.apm_config == config_name]
    for score_name in score_names:
      data_cell_scores = data_cell[data_cell.eval_score_name ==
                                         score_name].score
      scores[score_name] = sum(data_cell_scores) / len(data_cell_scores)

    result = {'scores': scores, 'params': {}}
    for param in params:
      result['params'][param] = config_json[param]
    results.append(result)
  return results


def _FindOptimalParameter(configs_and_scores, score_weighting):
  """Finds the config producing the maximal score.

  Goes through all configs in 'configs_and_scores' and returns the
  config that has the largest values of 'score_weighting' applied to
  its scores.

  An example score_weighting is 'lambda scores:
  sum(scores.values())'. A configs_and_score list is produced by
  '_ConfigurationAndScores'
  """

  min_score = float('-inf')
  best_params = None
  for config_and_score in configs_and_scores:
    current_score = score_weighting(config_and_score['scores'])
    if current_score > min_score:
      min_score = current_score
      best_params = config_and_score['params']
  return best_params

def main():
  # Init.
  logging.basicConfig(level=logging.DEBUG)  # TODO(alessio): INFO once debugged.
  parser = _InstanceArgumentsParser()
  args = parser.parse_args()

  # Get the scores.
  src_path = collect_data.ConstructSrcPath(args)
  logging.debug(src_path)
  scores_data_frame = collect_data.FindScores(src_path, args)
  all_scores = _ConfigurationAndScores(scores_data_frame,
                                       ["-" + x for x in args.params],
                                       args.config_dir)

  opt_param = _FindOptimalParameter(all_scores,
                                    lambda scores: sum(scores.values()))

  logging.info(opt_param)


if __name__ == "__main__":
  main()
