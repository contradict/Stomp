# encoding: ascii-8bit

# Copyright 2014 Ball Aerospace & Technologies Corp.
# All Rights Reserved.
#
# This program is free software; you can modify and/or redistribute it
# under the terms of the GNU General Public License
# as published by the Free Software Foundation; version 3 with
# attribution addendums as found in the LICENSE.txt

require 'cosmos'
require 'cosmos/tools/data_viewer/data_viewer_component'

module Cosmos

  # DumpComponent class
  #
  # This class displays packets as raw hex values
  #
  class DebugMessageComponent < DataViewerComponent

    # Processes the given packet. No gui interaction should be done in this
    # method. Override this method for other components.
    def process_packet (packet)
      processed_text = ''
      processed_text << packet.read('MSG') << "\n"
      # Ensure that queue does not grow infinitely while paused
      if @processed_queue.length < 1000
        @processed_queue << processed_text
      end
    end

  end

end # module Cosmos
