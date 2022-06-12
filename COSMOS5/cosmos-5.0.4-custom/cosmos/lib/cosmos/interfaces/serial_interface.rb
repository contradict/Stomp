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

require 'cosmos/interfaces/stream_interface'
require 'cosmos/streams/serial_stream'

module Cosmos
  # Provides a base class for interfaces that use serial ports
  class SerialInterface < StreamInterface
    # Creates a serial interface which uses the specified stream protocol.
    #
    # @param write_port_name [String] The name of the serial port to write
    # @param read_port_name [String] The name of the serial port to read
    # @param baud_rate [Integer] The serial port baud rate
    # @param parity [Symbol] The parity which is normally :NONE.
    #   Must be one of :NONE, :EVEN, or :ODD.
    # @param stop_bits [Integer] The number of stop bits which is normally 1.
    # @param write_timeout [Integer] The number of seconds to attempt the write
    #   before aborting
    # @param read_timeout [Integer] The number of seconds to attempt to read
    #   data from the serial port before aborting
    # @param protocol_type [String] Combined with 'Protocol' to resolve
    #   to a COSMOS protocol class
    # @param protocol_args [Array] Arguments to pass to the protocol constructor
    def initialize(write_port_name,
                   read_port_name,
                   baud_rate,
                   parity,
                   stop_bits,
                   write_timeout,
                   read_timeout,
                   protocol_type = nil,
                   *protocol_args)
      super(protocol_type, protocol_args)

      @write_port_name = ConfigParser.handle_nil(write_port_name)
      @read_port_name = ConfigParser.handle_nil(read_port_name)
      @baud_rate = baud_rate
      @parity = parity.to_s.intern
      @stop_bits = stop_bits
      @write_timeout = write_timeout
      @read_timeout = read_timeout
      @write_allowed = false unless @write_port_name
      @write_raw_allowed = false unless @write_port_name
      @read_allowed = false unless @read_port_name
      @flow_control = :NONE
      @data_bits = 8
    end

    # Creates a new {SerialStream} using the parameters passed in the constructor
    def connect
      @stream = SerialStream.new(
        @write_port_name,
        @read_port_name,
        @baud_rate,
        @parity,
        @stop_bits,
        @write_timeout,
        @read_timeout,
        @flow_control,
        @data_bits
      )
      super()
    end

    # Supported Options
    # FLOW_CONTROL - Flow control method NONE or RTSCTS. Defaults to NONE
    def set_option(option_name, option_values)
      super(option_name, option_values)
      case option_name.upcase
      when 'FLOW_CONTROL'
        @flow_control = option_values[0]
      when 'DATA_BITS'
        @data_bits = option_values[0].to_i
      end
    end
  end
end
