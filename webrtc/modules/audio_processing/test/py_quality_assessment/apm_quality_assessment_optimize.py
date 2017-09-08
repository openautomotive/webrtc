#!/usr/bin/env python
# Copyright (c) 2017 The WebRTC project authors. All Rights Reserved.
#
# Use of this source code is governed by a BSD-style license
# that can be found in the LICENSE file in the root of the source
# tree. An additional intellectual property rights grant can be found
# in the file PATENTS.  All contributing project authors may
# be found in the AUTHORS file in the root of the source tree.

import argparse
import collections
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

  parser.add_argument('-z', '--params_not_to_optimize', required=False, nargs='+',
                      default=[],
                      help=('parameters from `params` not to be optimized for'))

  return parser


# def _NormaliseScores(data_frame):
#   score_names = data_frame.eval_score_name.unique().tolist()

#   for score_name in score_names:
#     all_scores = data_frame[data_frame.eval_score_name == score_name].score
#     all_scores_sum =


def _ConfigurationAndScores(data_frame, params, params_not_to_optimize, config_dir):
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
  results = collections.defaultdict(list)
  config_names = data_frame['apm_config'].drop_duplicates().values.tolist()
  score_names = data_frame.eval_score_name.unique().tolist()

  # Normalize the scores
  normalization_constants = {}
  for score_name in score_names:
    scores = data_frame[data_frame.eval_score_name == score_name].score
    normalization_constants[score_name] = max(scores)

  params_to_optimize = [p for p in params if p not in params_not_to_optimize]
  ParamCombination = collections.namedtuple("ParamCombination", params_to_optimize)

  for config_name in config_names:
    config_json = data_access.AudioProcConfigFile.Load(
        os.path.join(config_dir, config_name + ".json"))
    scores = {}
    data_cell = data_frame[data_frame.apm_config == config_name]
    for score_name in score_names:
      data_cell_scores = data_cell[data_cell.eval_score_name ==
                                         score_name].score
      scores[score_name] = sum(data_cell_scores) / len(data_cell_scores)
      scores[score_name] /= normalization_constants[score_name]

    result = {'scores': scores, 'params': {}}
    config_optimize_params = {}
    for param in params:
      if param in params_to_optimize:
        config_optimize_params[param] = config_json['-' + param]
      else:
        result['params'][param] = config_json['-' + param]

    current_param_combination = ParamCombination(**config_optimize_params)
    results[current_param_combination].append(result)
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

  min_score = float('+inf')
  best_params = None
  for config in configs_and_scores:
    scores_and_params = configs_and_scores[config]
    current_score = score_weighting(scores_and_params)
    if current_score < min_score:
      min_score = current_score
      best_params = config
      logging.debug("New config!")
      logging.debug(current_score)
      logging.debug(config)
      logging.debug("--")
  return best_params

def weighting(scores_and_configs):
  BELOW_LIMIT_WEIGHT = 3
  AT_LIMIT_WEIGHT = 2
  ABOVE_LIMIT_WEIGHT = 1
  THD_WEIGHT = 3
  NOISE_WEIGHT = 3
  res = 0
  for x in scores_and_configs:
    gain = x['params']['agc2_fd_g']
    level_weight = BELOW_LIMIT_WEIGHT if gain < 0 else (
        AT_LIMIT_WEIGHT if gain == 0 else ABOVE_LIMIT_WEIGHT)
    res += x['scores']['thd']*THD_WEIGHT * level_weight
    res += max(x['scores']['saturation_distance'], 0) * level_weight
    res += x['scores']['noise']*2 * NOISE_WEIGHT * level_weight
  return res

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
                                       args.params,
                                       args.params_not_to_optimize,
                                       args.config_dir)

  opt_param = _FindOptimalParameter(all_scores, weighting)

  logging.info(opt_param)
  logging.info(all_scores[opt_param])


if __name__ == "__main__":
  main()
