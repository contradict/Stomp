# encoding: ascii-8bit

# Copyright 2021 Ball Aerospace & Technologies Corp.
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

require 'cosmos/models/target_model'
require 'cosmos/models/cvt_model'

module Cosmos
  module Api
    WHITELIST ||= []
    WHITELIST.concat([
                       'tlm',
                       'tlm_raw',
                       'tlm_formatted',
                       'tlm_with_units',
                       'tlm_variable',
                       'set_tlm',
                       'inject_tlm',
                       'override_tlm',
                       'normalize_tlm',
                       'get_tlm_buffer',
                       'get_tlm_packet',
                       'get_tlm_values',
                       'get_all_telemetry',
                       'get_telemetry',
                       'get_item',
                       'subscribe_packets',
                       'get_packet',
                       'get_all_tlm_info',
                       'get_tlm_cnt',
                       'get_packet_derived_items',
                     ])

    # Request a telemetry item from a packet.
    #
    # Accepts two different calling styles:
    #   tlm("TGT PKT ITEM")
    #   tlm('TGT','PKT','ITEM')
    #
    # Favor the first syntax where possible as it is more succinct.
    #
    # @param args [String|Array<String>] See the description for calling style
    # @param type [Symbol] Telemetry type, :RAW, :CONVERTED (default), :FORMATTED, or :WITH_UNITS
    # @return [Object] The telemetry value formatted as requested
    def tlm(*args, type: :CONVERTED, scope: $cosmos_scope, token: $cosmos_token)
      target_name, packet_name, item_name = tlm_process_args(args, 'tlm', scope: scope)
      authorize(permission: 'tlm', target_name: target_name, packet_name: packet_name, scope: scope, token: token)
      CvtModel.get_item(target_name, packet_name, item_name, type: type.intern, scope: scope)
    end

    # @deprecated Use tlm with type: :RAW
    def tlm_raw(*args, scope: $cosmos_scope, token: $cosmos_token)
      tlm(*args, type: :RAW, scope: scope, token: token)
    end

    # @deprecated Use tlm with type: :FORMATTED
    def tlm_formatted(*args, scope: $cosmos_scope, token: $cosmos_token)
      tlm(*args, type: :FORMATTED, scope: scope, token: token)
    end

    # @deprecated Use tlm with type: :WITH_UNITS
    def tlm_with_units(*args, scope: $cosmos_scope, token: $cosmos_token)
      tlm(*args, type: :WITH_UNITS, scope: scope, token: token)
    end

    # @deprecated Use tlm with type:
    def tlm_variable(*args, scope: $cosmos_scope, token: $cosmos_token)
      tlm(*args[0..-2], type: args[-1].intern, scope: scope, token: token)
    end

    # Set a telemetry item in the current value table.
    #
    # Note: If this is done while COSMOS is currently receiving telemetry,
    # this value could get overwritten at any time. Thus this capability is
    # best used for testing or for telemetry items that are not received
    # regularly through the target interface.
    #
    # Accepts two different calling styles:
    #   set_tlm("TGT PKT ITEM = 1.0")
    #   set_tlm('TGT','PKT','ITEM', 10.0)
    #
    # Favor the first syntax where possible as it is more succinct.
    #
    # @param args [String|Array<String>] See the description for calling style
    # @param type [Symbol] Telemetry type, :RAW, :CONVERTED (default), :FORMATTED, or :WITH_UNITS
    def set_tlm(*args, type: :CONVERTED, scope: $cosmos_scope, token: $cosmos_token)
      target_name, packet_name, item_name, value = set_tlm_process_args(args, __method__, scope: scope)
      authorize(permission: 'tlm_set', target_name: target_name, packet_name: packet_name, scope: scope, token: token)
      CvtModel.set_item(target_name, packet_name, item_name, value, type: type.intern, scope: scope)
    end

    # Injects a packet into the system as if it was received from an interface
    #
    # @param target_name[String] Target name of the packet
    # @param packet_name[String] Packet name of the packet
    # @param item_hash[Hash] Hash of item_name and value for each item you want to change from the current value table
    # @param type [Symbol] Telemetry type, :RAW, :CONVERTED (default), :FORMATTED, or :WITH_UNITS
    def inject_tlm(target_name, packet_name, item_hash = nil, type: :CONVERTED, scope: $cosmos_scope, token: $cosmos_token)
      authorize(permission: 'tlm_set', target_name: target_name, packet_name: packet_name, scope: scope, token: token)
      unless CvtModel::VALUE_TYPES.include?(type.intern)
        raise "Unknown type '#{type}' for #{target_name} #{packet_name}"
      end

      if item_hash
        # Check that the items exist ... exceptions are raised if not
        TargetModel.packet_items(target_name, packet_name, item_hash.keys, scope: scope)
      else
        # Check that the packet exists ... exceptions are raised if not
        TargetModel.packet(target_name, packet_name, scope: scope)
      end
      inject = {}
      inject['inject_tlm'] = true
      inject['target_name'] = target_name
      inject['packet_name'] = packet_name
      inject['item_hash'] = JSON.generate(item_hash) if item_hash
      inject['type'] = type

      InterfaceModel.all(scope: scope).each do |name, interface|
        if interface['target_names'].include? target_name
          Store.write_topic("{#{scope}__CMD}INTERFACE__#{interface['name']}", inject, '*', 100)
        end
      end
    end

    # Override the current value table such that a particular item always
    # returns the same value (for a given type) even when new telemetry
    # packets are received from the target.
    #
    # Accepts two different calling styles:
    #   override_tlm("TGT PKT ITEM = 1.0")
    #   override_tlm('TGT','PKT','ITEM', 10.0)
    #
    # Favor the first syntax where possible as it is more succinct.
    #
    # @param args The args must either be a string followed by a value or
    #   three strings followed by a value (see the calling style in the
    #   description).
    # @param type [Symbol] Telemetry type, :RAW, :CONVERTED (default), :FORMATTED, or :WITH_UNITS
    def override_tlm(*args, type: :CONVERTED, scope: $cosmos_scope, token: $cosmos_token)
      target_name, packet_name, item_name, value = set_tlm_process_args(args, __method__, scope: scope)
      authorize(permission: 'tlm_set', target_name: target_name, packet_name: packet_name, scope: scope, token: token)
      CvtModel.override(target_name, packet_name, item_name, value, type: type.intern, scope: scope)
    end

    # Normalize a telemetry item in a packet to its default behavior. Called
    # after override_tlm to restore standard processing.
    #
    # Accepts two different calling styles:
    #   normalize_tlm("TGT PKT ITEM")
    #   normalize_tlm('TGT','PKT','ITEM')
    #
    # Favor the first syntax where possible as it is more succinct.
    #
    # @param args The args must either be a string or three strings
    #   (see the calling style in the description).
    # @param type [Symbol] Telemetry type, :RAW, :CONVERTED (default), :FORMATTED, or :WITH_UNITS
    #   Also takes :ALL which means to normalize all telemetry types
    def normalize_tlm(*args, type: :ALL, scope: $cosmos_scope, token: $cosmos_token)
      target_name, packet_name, item_name = tlm_process_args(args, __method__, scope: scope)
      authorize(permission: 'tlm_set', target_name: target_name, packet_name: packet_name, scope: scope, token: token)
      CvtModel.normalize(target_name, packet_name, item_name, type: type.intern, scope: scope)
    end

    # Returns the raw buffer for a telemetry packet.
    #
    # @param target_name [String] Name of the target
    # @param packet_name [String] Name of the packet
    # @return [String] last telemetry packet buffer
    def get_tlm_buffer(target_name, packet_name, scope: $cosmos_scope, token: $cosmos_token)
      authorize(permission: 'tlm', target_name: target_name, packet_name: packet_name, scope: scope, token: token)
      TargetModel.packet(target_name, packet_name, scope: scope)
      topic = "#{scope}__TELEMETRY__{#{target_name}}__#{packet_name}"
      msg_id, msg_hash = Store.instance.read_topic_last(topic)
      return msg_hash['buffer'].b if msg_id # Return as binary

      nil
    end

    # Returns all the values (along with their limits state) for a packet.
    #
    # @param target_name [String] Name of the target
    # @param packet_name [String] Name of the packet
    # @param type [Symbol] Types returned, :RAW, :CONVERTED (default), :FORMATTED, or :WITH_UNITS
    # @return (see Cosmos::Packet#read_all_with_limits_states)
    def get_tlm_packet(target_name, packet_name, type: :CONVERTED, scope: $cosmos_scope, token: $cosmos_token)
      authorize(permission: 'tlm', target_name: target_name, packet_name: packet_name, scope: scope, token: token)
      TargetModel.packet(target_name, packet_name, scope: scope)
      case type.intern
      when :RAW
        desired_item_type = ''
      when :CONVERTED
        desired_item_type = 'C'
      when :FORMATTED
        desired_item_type = 'F'
      when :WITH_UNITS
        desired_item_type = 'U'
      else
        raise "Unknown type '#{type}' for #{target_name} #{packet_name}"
      end
      result_hash = {}
      topic = "#{scope}__DECOM__{#{target_name}}__#{packet_name}"
      msg_id, msg_hash = Store.instance.read_topic_last(topic)
      if msg_id
        json = msg_hash['json_data']
        hash = JSON.parse(json)
        # This should be ordered as desired... need to verify
        hash.each do |key, value|
          split_key = key.split("__")
          item_name = split_key[0].to_s
          item_type = split_key[1]
          result_hash[item_name] ||= [item_name]
          if item_type == 'L'
            result_hash[item_name][2] = value
          else
            if item_type.to_s <= desired_item_type.to_s
              if desired_item_type == 'F' or desired_item_type == 'U'
                result_hash[item_name][1] = value.to_s
              else
                result_hash[item_name][1] = value
              end
            end
          end
        end
        return result_hash.values
      else
        return nil
      end
    end

    # Returns all the item values (along with their limits state). The items
    # can be from any target and packet and thus must be fully qualified with
    # their target and packet names.
    #
    # @version 5.0.0
    # @param items [Array<String>] Array of items consisting of 'tgt__pkt__item__type'
    # @return [Array<Object, Symbol>]
    #   Array consisting of the item value and limits state
    #   given as symbols such as :RED, :YELLOW, :STALE
    def get_tlm_values(items, scope: $cosmos_scope, token: $cosmos_token)
      if !items.is_a?(Array) || !items[0].is_a?(String)
        raise ArgumentError, "items must be array of strings: ['TGT__PKT__ITEM__TYPE', ...]"
      end

      items.each_with_index do |item, index|
        target_name, packet_name, item_name, item_type = item.split('__')
        if packet_name == 'LATEST'
          _, packet_name, _ = tlm_process_args([target_name, packet_name, item_name], 'get_tlm_values', scope: scope) # Figure out which packet is LATEST
          items[index] = "#{target_name}__#{packet_name}__#{item_name}__#{item_type}" # Replace LATEST with the real packet name
        end
        authorize(permission: 'tlm', target_name: target_name, packet_name: packet_name, scope: scope, token: token)
      end
      CvtModel.get_tlm_values(items, scope: scope)
    end

    # Returns an array of all the telemetry packet hashes
    #
    # @since 5.0.0
    # @param target_name [String] Name of the target
    # @return [Array<Hash>] Array of all telemetry packet hashes
    def get_all_telemetry(target_name, scope: $cosmos_scope, token: $cosmos_token)
      authorize(permission: 'tlm', target_name: target_name, scope: scope, token: token)
      TargetModel.packets(target_name, scope: scope)
    end

    # Returns a telemetry packet hash
    #
    # @since 5.0.0
    # @param target_name [String] Name of the target
    # @param packet_name [String] Name of the packet
    # @return [Hash] Telemetry packet hash
    def get_telemetry(target_name, packet_name, scope: $cosmos_scope, token: $cosmos_token)
      authorize(permission: 'tlm', target_name: target_name, packet_name: packet_name, scope: scope, token: token)
      TargetModel.packet(target_name, packet_name, scope: scope)
    end

    # Returns a telemetry packet item hash
    #
    # @since 5.0.0
    # @param target_name [String] Name of the target
    # @param packet_name [String] Name of the packet
    # @param item_name [String] Name of the packet
    # @return [Hash] Telemetry packet item hash
    def get_item(target_name, packet_name, item_name, scope: $cosmos_scope, token: $cosmos_token)
      authorize(permission: 'tlm', target_name: target_name, packet_name: packet_name, scope: scope, token: token)
      TargetModel.packet_item(target_name, packet_name, item_name, scope: scope)
    end

    # Subscribe to a list of packets. An ID is returned which is passed to
    # get_packet(id) to yield packets back to a block.
    #
    # @param packets [Array<Array<String, String>>] Array of arrays consisting of target name, packet name
    # @return [String] ID which should be passed to get_packet
    def subscribe_packets(packets, scope: $cosmos_scope, token: $cosmos_token)
      if !packets.is_a?(Array) || !packets[0].is_a?(Array)
        raise ArgumentError, "packets must be nested array: [['TGT','PKT'],...]"
      end

      result = [Time.now.to_nsec_from_epoch]
      packets.each do |target_name, packet_name|
        authorize(permission: 'tlm', target_name: target_name, packet_name: packet_name, scope: scope, token: token)
        result << "#{scope}__DECOM__{#{target_name}}__#{packet_name}"
      end
      result.join("\n")
    end
    # Alias the singular as well since that matches COSMOS 4
    alias subscribe_packet subscribe_packets

    # Get a packet which was previously subscribed to by subscribe_packet.
    # This method takes a block and yields back packet hashes.
    def get_packet(id, scope: $cosmos_scope, token: $cosmos_token)
      authorize(permission: 'tlm', scope: scope, token: token)
      offset, *topics = id.split("\n")
      offsets = []
      # Create a common array of offsets for each of the topics
      topics.length.times do
        offsets << (offset.to_i / 1_000_000).to_s + '-0'
      end
      Topic.read_topics(topics, offsets) do |topic, msg_id, msg_hash, redis|
        json_hash = JSON.parse(msg_hash['json_data'])
        msg_hash.delete('json_data')
        yield msg_hash.merge(json_hash)
      end
    end

    # Get the receive count for a telemetry packet
    #
    # @param target_name [String] Name of the target
    # @param packet_name [String] Name of the packet
    # @return [Numeric] Receive count for the telemetry packet
    def get_tlm_cnt(target_name, packet_name, scope: $cosmos_scope, token: $cosmos_token)
      authorize(permission: 'system', target_name: target_name, packet_name: packet_name, scope: scope, token: token)
      TargetModel.packet(target_name, packet_name, scope: scope)
      _get_cnt("#{scope}__TELEMETRY__{#{target_name}}__#{packet_name}")
    end

    # Get information on all telemetry packets
    #
    # @return [Array<String, String, Numeric>] Receive count for all telemetry
    def get_all_tlm_info(scope: $cosmos_scope, token: $cosmos_token)
      get_all_cmd_tlm_info("TELEMETRY", scope: scope, token: token)
    end

    # Get the list of derived telemetry items for a packet
    #
    # @param target_name [String] Target name
    # @param packet_name [String] Packet name
    # @return [Array<String>] All of the ignored telemetry items for a packet.
    def get_packet_derived_items(target_name, packet_name, scope: $cosmos_scope, token: $cosmos_token)
      authorize(permission: 'tlm', target_name: target_name, packet_name: packet_name, scope: scope, token: token)
      packet = TargetModel.packet(target_name, packet_name, scope: scope)
      return packet['items'].select { |item| item['data_type'] == 'DERIVED' }.map { |item| item['name'] }
    end

    # PRIVATE

    def tlm_process_args(args, function_name, scope: $cosmos_scope, token: $cosmos_token)
      case args.length
      when 1
        target_name, packet_name, item_name = extract_fields_from_tlm_text(args[0])
      when 3
        target_name = args[0]
        packet_name = args[1]
        item_name = args[2]
      else
        # Invalid number of arguments
        raise "ERROR: Invalid number of arguments (#{args.length}) passed to #{function_name}()"
      end
      if packet_name == 'LATEST'
        latest = -1
        TargetModel.packets(target_name, scope: scope).each do |packet|
          item = packet['items'].find { |item| item['name'] == item_name }
          if item
            _, msg_hash = Store.instance.get_oldest_message("#{scope}__DECOM__{#{target_name}}__#{packet['packet_name']}")
            if msg_hash && msg_hash['time'] && msg_hash['time'].to_i > latest
              packet_name = packet['packet_name']
              latest = msg_hash['time'].to_i
            end
          end
        end
        raise "Item '#{target_name} LATEST #{item_name}' does not exist" if latest == -1
      else
        # Determine if this item exists, it will raise appropriate errors if not
        TargetModel.packet_item(target_name, packet_name, item_name, scope: scope)
      end

      return [target_name, packet_name, item_name]
    end

    def set_tlm_process_args(args, function_name, scope: $cosmos_scope, token: $cosmos_token)
      case args.length
      when 1
        target_name, packet_name, item_name, value = extract_fields_from_set_tlm_text(args[0])
      when 4
        target_name = args[0]
        packet_name = args[1]
        item_name = args[2]
        value = args[3]
      else
        # Invalid number of arguments
        raise "ERROR: Invalid number of arguments (#{args.length}) passed to #{function_name}()"
      end
      # Determine if this item exists, it will raise appropriate errors if not
      TargetModel.packet_item(target_name, packet_name, item_name, scope: scope)

      return [target_name, packet_name, item_name, value]
    end
  end
end
