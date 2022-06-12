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
require 'cosmos/models/metric_model'

module Cosmos
  describe MetricModel do
    before(:each) do
      mock_redis()
    end

    describe "self.all" do
      it "returns all the metrics" do
        model = MetricModel.new(name: "foo", scope: "scope", metric_name: "foobar", label_list: ["tacocat"])
        model.create(force: true)
        all = MetricModel.all(scope: "scope")
        expect(all.empty?).to eql(false)
        expect(all["foo"].empty?).to eql(false)
        expect(all["foo"]["scope"]).to eql(nil)
        expect(all["foo"]["metric_name"]).to eql("foobar")
        expect(all["foo"]["label_list"]).to eql(["tacocat"])
      end
    end

    describe "as_json" do
      it "encodes all the input parameters" do
        model = MetricModel.new(name: "foo", scope: "scope", metric_name: "foobar", label_list: ["tacocat"])
        json = model.as_json
        expect(json["metric_name"]).to eql("foobar")
        expect(json["label_list"]).to eql(["tacocat"])
      end
    end
  end
end
