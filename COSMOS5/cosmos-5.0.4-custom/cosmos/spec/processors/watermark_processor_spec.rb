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

require 'spec_helper'
require 'cosmos'
require 'cosmos/processors/watermark_processor'

module Cosmos
  describe WatermarkProcessor do
    describe "initialize" do
      it "takes an item_name and value_type" do
        p = WatermarkProcessor.new('TEST', 'RAW')
        expect(p.value_type).to eql :RAW
        expect(p.instance_variable_get("@item_name")).to eql 'TEST'
      end
    end

    describe "call and reset" do
      it "generates a high and low water mark" do
        p = WatermarkProcessor.new('TEST', 'RAW')
        packet = Packet.new("tgt", "pkt")
        packet.append_item("TEST", 8, :UINT)
        packet.buffer = "\x01"
        p.call(packet, packet.buffer)
        expect(p.results[:HIGH_WATER]).to eql 1
        expect(p.results[:LOW_WATER]).to eql 1
        packet.buffer = "\x02"
        p.call(packet, packet.buffer)
        expect(p.results[:HIGH_WATER]).to eql 2
        expect(p.results[:LOW_WATER]).to eql 1
        packet.buffer = "\x00"
        p.call(packet, packet.buffer)
        expect(p.results[:HIGH_WATER]).to eql 2
        expect(p.results[:LOW_WATER]).to eql 0
        p.reset
        expect(p.results[:HIGH_WATER]).to eql nil
        expect(p.results[:LOW_WATER]).to eql nil
      end
    end
  end
end
