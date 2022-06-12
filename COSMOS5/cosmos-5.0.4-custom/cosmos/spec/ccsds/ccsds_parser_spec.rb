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
require 'cosmos/ccsds/ccsds_parser'

module Cosmos
  describe CcsdsParser do
    before(:each) do
      @parser = CcsdsParser.new
    end

    describe "initialize" do
      it "sets the state to :READY" do
        expect(@parser.state).to eql :READY
      end

      it "clears the in_progress_data" do
        expect(@parser.in_progress_data).to eql ""
      end
    end

    describe "reset" do
      it "sets the state to :READY" do
        @parser.reset
        expect(@parser.state).to eql :READY
      end

      it "clears the in_progress_data" do
        @parser.reset
        expect(@parser.in_progress_data).to eql ""
      end
    end

    describe "unsegment_packet" do
      it "returns standalone CCSDS packets" do
        pkt = CcsdsPacket.new
        pkt.write('CCSDSSEQFLAGS', CcsdsPacket::STANDALONE)
        pkt.write('CCSDSDATA', "\x01\x02\x03\x04")
        expect(@parser.unsegment_packet(pkt)).to eql pkt.buffer
      end

      it "combines two CCSDS packets" do
        pkt1 = CcsdsPacket.new
        pkt1.write('CCSDSSEQFLAGS', CcsdsPacket::FIRST)
        pkt1.write('CCSDSSEQCNT', 0)
        pkt1.write('CCSDSDATA', "\x01\x02\x03\x04")
        pkt2 = CcsdsPacket.new
        pkt2.write('CCSDSSEQFLAGS', CcsdsPacket::LAST)
        pkt2.write('CCSDSSEQCNT', 1)
        pkt2.write('CCSDSDATA', "\x05\x06\x07\x08")
        expect(@parser.unsegment_packet(pkt1)).to eql nil
        expect(@parser.unsegment_packet(pkt2)).to eql pkt1.buffer + pkt2.read("CCSDSDATA")
      end

      it "combines three CCSDS packets" do
        pkt1 = CcsdsPacket.new
        pkt1.write('CCSDSSEQFLAGS', CcsdsPacket::FIRST)
        pkt1.write('CCSDSSEQCNT', 0)
        pkt1.write('CCSDSDATA', "\x01\x02\x03\x04")
        pkt2 = CcsdsPacket.new
        pkt2.write('CCSDSSEQFLAGS', CcsdsPacket::CONTINUATION)
        pkt2.write('CCSDSSEQCNT', 1)
        pkt2.write('CCSDSDATA', "\x05\x06\x07\x08")
        pkt3 = CcsdsPacket.new
        pkt3.write('CCSDSSEQFLAGS', CcsdsPacket::LAST)
        pkt3.write('CCSDSSEQCNT', 2)
        pkt3.write('CCSDSDATA', "\x09\x0A\x0B\x0C")
        expect(@parser.unsegment_packet(pkt1)).to eql nil
        expect(@parser.unsegment_packet(pkt2)).to eql nil
        expect(@parser.unsegment_packet(pkt3)).to eql pkt1.buffer + pkt2.read("CCSDSDATA") + pkt3.read("CCSDSDATA")
      end

      it "complains with an initial continuation packet" do
        pkt = CcsdsPacket.new
        pkt.write('CCSDSSEQFLAGS', CcsdsPacket::CONTINUATION)
        expect { @parser.unsegment_packet(pkt) }.to raise_error(CcsdsParser::CcsdsSegmentationError, "Unexpected continuation packet")
      end

      it "complains with an initial last packet" do
        pkt = CcsdsPacket.new
        pkt.write('CCSDSSEQFLAGS', CcsdsPacket::LAST)
        expect { @parser.unsegment_packet(pkt) }.to raise_error(CcsdsParser::CcsdsSegmentationError, "Unexpected last packet")
      end

      it "complains with an out of order continuation sequence count" do
        pkt1 = CcsdsPacket.new
        pkt1.write('CCSDSSEQFLAGS', CcsdsPacket::FIRST)
        pkt1.write('CCSDSSEQCNT', 0)
        pkt1.write('CCSDSDATA', "\x01\x02\x03\x04")
        pkt2 = CcsdsPacket.new
        pkt2.write('CCSDSSEQFLAGS', CcsdsPacket::CONTINUATION)
        pkt2.write('CCSDSSEQCNT', 2)
        pkt2.write('CCSDSDATA', "\x05\x06\x07\x08")
        expect(@parser.unsegment_packet(pkt1)).to eql nil
        expect { @parser.unsegment_packet(pkt2) }.to raise_error(CcsdsParser::CcsdsSegmentationError, /Missing packet/)
      end

      it "complains with an out of order last sequence count" do
        pkt1 = CcsdsPacket.new
        pkt1.write('CCSDSSEQFLAGS', CcsdsPacket::FIRST)
        pkt1.write('CCSDSSEQCNT', 0)
        pkt1.write('CCSDSDATA', "\x01\x02\x03\x04")
        pkt2 = CcsdsPacket.new
        pkt2.write('CCSDSSEQFLAGS', CcsdsPacket::LAST)
        pkt2.write('CCSDSSEQCNT', 2)
        pkt2.write('CCSDSDATA', "\x05\x06\x07\x08")
        expect(@parser.unsegment_packet(pkt1)).to eql nil
        expect { @parser.unsegment_packet(pkt2) }.to raise_error(CcsdsParser::CcsdsSegmentationError, /Missing packet/)
      end

      it "complains with a first in the middle of processing" do
        pkt1 = CcsdsPacket.new
        pkt1.write('CCSDSSEQFLAGS', CcsdsPacket::FIRST)
        pkt1.write('CCSDSSEQCNT', 0)
        pkt1.write('CCSDSDATA', "\x01\x02\x03\x04")
        pkt2 = CcsdsPacket.new
        pkt2.write('CCSDSSEQFLAGS', CcsdsPacket::FIRST)
        pkt2.write('CCSDSSEQCNT', 1)
        pkt2.write('CCSDSDATA', "\x05\x06\x07\x08")
        expect(@parser.unsegment_packet(pkt1)).to eql nil
        expect { @parser.unsegment_packet(pkt2) }.to raise_error(CcsdsParser::CcsdsSegmentationError, "Unexpected first packet")
      end

      it "complains with a standalone in the middle of processing" do
        pkt1 = CcsdsPacket.new
        pkt1.write('CCSDSSEQFLAGS', CcsdsPacket::FIRST)
        pkt1.write('CCSDSSEQCNT', 0)
        pkt1.write('CCSDSDATA', "\x01\x02\x03\x04")
        pkt2 = CcsdsPacket.new
        pkt2.write('CCSDSSEQFLAGS', CcsdsPacket::STANDALONE)
        pkt2.write('CCSDSDATA', "\x05\x06\x07\x08")
        expect(@parser.unsegment_packet(pkt1)).to eql nil
        expect { @parser.unsegment_packet(pkt2) }.to raise_error(CcsdsParser::CcsdsSegmentationError, "Unexpected standalone packet")
      end
    end
  end
end
