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

require 'cosmos/models/trigger_group_model'
require 'cosmos/models/trigger_model'
require 'cosmos/topics/autonomic_topic'

class TriggerGroupController < ApplicationController
  def initialize
    @model_class = Cosmos::TriggerGroupModel
  end

  # Returns an array/list of trigger values in json.
  #
  # scope [String] the scope of the group, `TEST`
  # @return [String] the array of triggers converted into json format
  def index
    begin
      authorize(permission: 'system', scope: params[:scope], token: request.headers['HTTP_AUTHORIZATION'])
    rescue Cosmos::AuthError => e
      render(:json => { :status => 'error', :message => e.message }, :status => 401) and return
    rescue Cosmos::ForbiddenError => e
      render(:json => { :status => 'error', :message => e.message }, :status => 403) and return
    end
    begin
      ret = Array.new
      trigger_groups = @model_class.all(scope: params[:scope])
      trigger_groups.each do |_, trigger_group|
        ret << trigger_group
      end
      render :json => ret, :status => 200
    rescue StandardError => e
      render :json => { :status => 'error', :message => e.message, 'type' => e.class, 'backtrace' => e.backtrace }, :status => 500
    end
  end

  # Returns a single trigger in json.
  #
  # name [String] the trigger name, `systemGroup`
  # scope [String] the scope of the group, `TEST`
  # @return [String] the array of triggers converted into json format.
  def show
    begin
      authorize(permission: 'system', scope: params[:scope], token: request.headers['HTTP_AUTHORIZATION'])
    rescue Cosmos::AuthError => e
      render(:json => { :status => 'error', :message => e.message }, :status => 401) and return
    rescue Cosmos::ForbiddenError => e
      render(:json => { :status => 'error', :message => e.message }, :status => 403) and return
    end
    begin
      model = @model_class.get(name: params[:name], scope: params[:scope])
      if model.nil?
        render :json => { :status => 'error', :message => 'not found' }, :status => 404
        return
      end
      render :json => model.as_json(), :status => 200
    rescue Cosmos::TriggerGroupInputError => e
      render :json => { :status => 'error', :message => e.message, 'type' => e.class }, :status => 400
    rescue StandardError => e
      render :json => { :status => 'error', :message => e.message, 'type' => e.class, 'backtrace' => e.backtrace }, :status => 500
    end
  end

  # Create a new group and return the object/hash of the trigger in json.
  #
  # scope [String] the scope of the group, `TEST`
  # json [String] The json of the event (see #trigger_model)
  # @return [String] the trigger converted into json format
  # Request Headers
  #```json
  #  {
  #    "Authorization": "token/password",
  #    "Content-Type": "application/json"
  #  }
  #```
  # Request Post Body
  #```json
  #  {
  #    "name": "systemGroup",
  #    "color": "#FFFFFF"
  #  }
  #```
  def create
    begin
      authorize(permission: 'run_script', scope: params[:scope], token: request.headers['HTTP_AUTHORIZATION'])
    rescue Cosmos::AuthError => e
      render(:json => { :status => 'error', :message => e.message }, :status => 401) and return
    rescue Cosmos::ForbiddenError => e
      render(:json => { :status => 'error', :message => e.message }, :status => 403) and return
    end
    begin
      model = @model_class.new(name: params[:name], color: params[:color], scope: params[:scope])
      model.create()
      model.deploy()
      render :json => model.as_json, :status => 201
    rescue Cosmos::TriggerGroupInputError => e
      render :json => { :status => 'error', :message => e.message, 'type' => e.class }, :status => 400
    rescue Cosmos::TriggerGroupError => e
      render :json => { :status => 'error', :message => e.message, 'type' => e.class }, :status => 418
    rescue StandardError => e
      render :json => { :status => 'error', :message => e.message, 'type' => e.class, 'backtrace' => e.backtrace }, :status => 500
    end
  end

  # Change the color returns object/hash of the trigger group in json.
  #
  # group [String] the trigger group name, `system42`
  # scope [String] the scope of the group, `TEST`
  # json [String] The json of the color (see below)
  # @return [String] the timeline converted into json format
  # Request Headers
  #```json
  #  {
  #    "Authorization": "token/password",
  #    "Content-Type": "application/json"
  #  }
  #```
  # Request Post Body
  #```json
  #  {
  #    "color": "#FFFFFF"
  #  }
  #```
  def color
    begin
      authorize(permission: 'run_script', scope: params[:scope], token: request.headers['HTTP_AUTHORIZATION'])
    rescue Cosmos::AuthError => e
      render(:json => { :status => 'error', :message => e.message }, :status => 401) and return
    rescue Cosmos::ForbiddenError => e
      render(:json => { :status => 'error', :message => e.message }, :status => 403) and return
    end
    model = @model_class.get(name: params[:group], scope: params[:scope])
    if model.nil?
      render :json => {
        'status' => 'error',
        'message' => "failed to find trigger group: #{params[:group]}",
      }, :status => 404
      return
    end
    begin
      model.update_color(color: params['color'])
      model.update()
      render :json => model.as_json, :status => 200
    rescue Cosmos::ReactionInputError => e
      render :json => { :status => 'error', :message => e.message, 'type' => e.class }, :status => 400
    rescue StandardError => e
      render :json => { :status => 'error', :message => e.message, 'type' => e.class, 'backtrace' => e.backtrace }, :status => 500
    end
  end

  # Activate all triggers in group.
  #
  # group [String] the trigger group name, `system42`
  # scope [String] the scope of the group, `TEST`
  # json [String] Empty json object (see below)
  # @return [Array] Of triggers in group activated converted into json format
  # Request Headers
  #```json
  #  {
  #    "Authorization": "token/password",
  #    "Content-Type": "application/json"
  #  }
  #```
  # Request Post Body
  #```json
  #  {}
  #```
  def activate
    begin
      authorize(permission: 'run_script', scope: params[:scope], token: request.headers['HTTP_AUTHORIZATION'])
    rescue Cosmos::AuthError => e
      render(:json => { :status => 'error', :message => e.message }, :status => 401) and return
    rescue Cosmos::ForbiddenError => e
      render(:json => { :status => 'error', :message => e.message }, :status => 403) and return
    end
    model = @model_class.get(name: params[:group], scope: params[:scope])
    if model.nil?
      render :json => {
        'status' => 'error',
        'message' => "failed to find trigger group: #{params[:group]}",
      }, :status => 404
      return
    end
    ret = Array.new
    begin
      triggers = TriggerModel.all(group: params[:group], scope: params[:scope])
      triggers.each do | t_name, t_hash |
        trigger = TriggerModel.from_json(t_hash, name: t_hash['name'], scope: t_hash['scope'])
        trigger.activate() unless trigger.active
        ret << t_name
      end
      render :json => ret.as_json, :status => 200
    rescue Cosmos::TriggerError => e
      render :json => { :status => 'error', :message => e.message, 'type' => e.class }, :status => 400
    rescue StandardError => e
      render :json => { :status => 'error', :message => e.message, 'type' => e.class, 'backtrace' => e.backtrace }, :status => 500
    end
  end

  # Deactivate all triggers in group.
  #
  # group [String] the trigger group name, `system42`
  # scope [String] the scope of the group, `TEST`
  # json [String] Empty json object (see below)
  # @return [Array] Of triggers in group activated into json format
  # Request Headers
  #```json
  #  {
  #    "Authorization": "token/password",
  #    "Content-Type": "application/json"
  #  }
  #```
  # Request Post Body
  #```json
  #  {}
  #```
  def deactivate
    begin
      authorize(permission: 'run_script', scope: params[:scope], token: request.headers['HTTP_AUTHORIZATION'])
    rescue Cosmos::AuthError => e
      render(:json => { :status => 'error', :message => e.message }, :status => 401) and return
    rescue Cosmos::ForbiddenError => e
      render(:json => { :status => 'error', :message => e.message }, :status => 403) and return
    end
    model = @model_class.get(name: params[:group], scope: params[:scope])
    if model.nil?
      render :json => {
        'status' => 'error',
        'message' => "failed to find trigger group: #{params[:group]}",
      }, :status => 404
      return
    end
    ret = Array.new
    begin
      triggers = TriggerModel.all(group: params[:group], scope: params[:scope])
      triggers.each do | t_name, t_hash |
        trigger = TriggerModel.from_json(t_hash, name: t_hash['name'], scope: t_hash['scope'])
        trigger.deactivate() if trigger.active
        ret << t_name
      end
      render :json => ret.as_json, :status => 200
    rescue Cosmos::TriggerError => e
      render :json => { :status => 'error', :message => e.message, 'type' => e.class }, :status => 400
    rescue StandardError => e
      render :json => { :status => 'error', :message => e.message, 'type' => e.class, 'backtrace' => e.backtrace }, :status => 500
    end
  end

  # Removes an trigger trigger by name
  #
  # group [String] the trigger name, `systemGroup`
  # scope [String] the scope of the trigger, `TEST`
  # id [String] the score or id of the trigger, `1620248449`
  # @return [String] object/hash converted into json format but with a 204 no-content status code
  # Request Headers
  #```json
  #  {
  #    "Authorization": "token/password",
  #    "Content-Type": "application/json"
  #  }
  #```
  def destroy
    begin
      authorize(permission: 'run_script', scope: params[:scope], token: request.headers['HTTP_AUTHORIZATION'])
    rescue Cosmos::AuthError => e
      render(:json => { :status => 'error', :message => e.message }, :status => 401) and return
    rescue Cosmos::ForbiddenError => e
      render(:json => { :status => 'error', :message => e.message }, :status => 403) and return
    end
    begin
      @model_class.delete(name: params[:group], scope: params[:scope])
      render :json => { :delete => true, :group => params[:group] }, :status => 204
    rescue Cosmos::TriggerGroupInputError => e
      render :json => { :status => 'error', :message => e.message, 'type' => e.class }, :status => 404
    rescue Cosmos::TriggerGroupError => e
      render :json => { :status => 'error', :message => e.message, 'type' => e.class }, :status => 400
    rescue StandardError => e
      render :json => { :status => 'error', :message => e.message, 'type' => e.class, 'backtrace' => e.backtrace }, :status => 500
    end
  end

end
