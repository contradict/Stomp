# encoding: ascii-8bit

# Copyright 2022 Ball Aerospace & Technologies Corp.
# All Rights Reserved.
#
# This program is free software; you can modify and/or redistribute it
# under the terms of the GNU Affero General Public License
# as published by the Free Software Foundation; version 3 with
# attribution addendums as found in the LICENSE.txt
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# This program may also be used under the terms of a commercial or
# enterprise edition license of COSMOS if purchased from the
# copyright holder

require 'cosmos/config/config_parser'
require 'cosmos/system/system'
require 'fileutils'
require 'cosmos/utilities/s3'

module Cosmos
  # Handles writing message logs to a file
  class MessageLog
    # @return [String] The name of the message log file. Empty string until the
    #   write or start methods are called at which point it is set to the
    #   filename. Retains the last filename even after stop is called.
    attr_reader :filename

    # @param tool_name [String] The name of the tool creating the message log.
    #   This will be inserted into the message log filename to help identify it.
    # @param log_dir [String] The filesystem path to store the message log file.
    def initialize(tool_name, log_dir, scope:)
      @remote_log_directory = "#{scope}/tool_logs/#{tool_name}/"
      @tool_name = tool_name
      @log_dir = log_dir
      @filename = ''
      @file = nil
      @start_day = nil
      @mutex = Mutex.new
    end

    # Ensures the log file is opened and ready to write. It then writes the
    # message to the log and flushes it to force the write.
    #
    # @param message [String] Message to write to the log
    def write(message, flush = false)
      @mutex.synchronize do
        if @file.nil? or @file.closed? or (not File.exist?(@filename))
          start(false)
        end

        @file.write(message)
        @file.flush if flush
      end
    end

    # Closes the message log and marks it read only
    def stop(take_mutex = true, s3_object_metadata: {})
      @mutex.lock if take_mutex
      if @file and not @file.closed?
        @file.close
        File.chmod(0444, @filename)
        s3_key = File.join(@remote_log_directory, @start_day, File.basename(@filename))
        begin
          thread = S3Utilities.move_log_file_to_s3(@filename, s3_key, metadata: s3_object_metadata)
          thread.join
        rescue StandardError => e
          Logger.error e.formatted
        end
      end
      @mutex.unlock if take_mutex
    end

    # Creates a new message log and sets the filename
    def start(take_mutex = true)
      @mutex.lock if take_mutex
      # Prevent starting files too fast
      sleep(0.1) until !File.exist?(File.join(@log_dir, File.build_timestamped_filename([@tool_name, 'messages'])))
      stop(false)
      timed_filename = File.build_timestamped_filename([@tool_name, 'messages'])
      @start_day = timed_filename[0..9].gsub("_", "") # YYYYMMDD
      @filename = File.join(@log_dir, timed_filename)
      @file = File.open(@filename, 'a')
      @mutex.unlock if take_mutex
    end
  end
end
