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
import matplotlib.pyplot as plt
import os
import re

import quality_assessment.data_access as data_access
import apm_quality_assessment_collect_data as collect_data

def _InstanceArgumentsParser():
  """Arguments parser factory.
  """
  parser = collect_data.InstanceArgumentsParser()
  parser.description=(
      'Shows boxplot of given score for different values of selected'
      'parameters. Can be used to compare scores by audioproc_f flag')

  parser.add_argument('-v', '--eval_score', required=True,
                      help=('Score name for constructing boxplots'))

  parser.add_argument('-n', '--config_dir', required=False,
                      help=('path to the folder with the configuration files'),
                      default='apm_configs')

  parser.add_argument('-z', '--params_to_plot', required=True, nargs='+',
                      help=('Parameter values on which to group scores'))

  # parser.add_argument('-z', '--param_to_select', required=True,
  #                     help=('Parameter values on which to compare'))

  return parser

def _FindMatchingParam(filter_params, config_json):
  matching = [x for x in filter_params if '-' + x in config_json]
  assert len(matching) == 1
  return matching[0]


def _FilterScoresByParams(data_frame, filter_params, score_name, config_dir):
  """TODO doc.
  Returns:
    dictionary, key is a param value, result is all scores for that param value.
  """
  logging.debug("Score name: " + str(score_name))
  results = collections.defaultdict(list)
  config_names = data_frame['apm_config'].drop_duplicates().values.tolist()

  for config_name in config_names:
    config_json = data_access.AudioProcConfigFile.Load(
        os.path.join(config_dir, config_name + ".json"))
    data_cell = data_frame[data_frame.apm_config == config_name]
    #logging.debug("Found " + str(data_cell))
    data_cell_scores = data_cell[data_cell.eval_score_name ==
                                         score_name].score

    # Exactly one of 'params_to_plot' must match:
    matching_param = _FindMatchingParam(filter_params, config_json)
    logging.debug("Adding " + str(data_cell_scores))
    results[config_json['-' + matching_param]] += list(data_cell_scores)
  return results


def main():
  # Init.
  logging.basicConfig(level=logging.DEBUG)  # TODO(alessio): INFO once debugged.
  parser = _InstanceArgumentsParser()
  args = parser.parse_args()

  # Get the scores.
  src_path = collect_data.ConstructSrcPath(args)
  logging.debug(src_path)
  scores_data_frame = collect_data.FindScores(src_path, args)

  # Filter the data by `args.params_to_plot`
  scores_filtered = _FilterScoresByParams(scores_data_frame,
                                          args.params_to_plot,
                                          args.eval_score,
                                          args.config_dir)

  # logging.debug("Scores filtered: " + str(scores_filtered))

  data_list = sorted(scores_filtered.items())
  data_values = [x for (_, x) in data_list]
  data_labels = [x for (x, _) in data_list]


  fig, axes = plt.subplots(nrows=1, ncols=1, figsize=(6, 6))
  axes.boxplot(data_values, labels=data_labels)
  axes.set_ylabel(args.eval_score)
  axes.set_xlabel('/'.join(args.params_to_plot))
  plt.show()

  #return scores_filtered
  # logging.info(opt_param)
  # logging.info(all_scores[opt_param])

if __name__ == "__main__":
  main()
