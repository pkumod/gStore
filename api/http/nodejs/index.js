const request = require('request-promise');

class GStoreClient {
  constructor(username = '', password = '', host = '') {
    this.username = username;
    this.password = password;
    this.host = host;
  }

  basicTemplate(operation = '', otherParams = '') {
    return `${this.host}/${encodeURIComponent(
      `?operation=${operation}&username=${this.username}&password=${
        this.password
      }${otherParams ? '&' + otherParams : ''}`
    )}`;
  }

  // query given database with sparql
  async query(db = '', sparql = '') {
    // 请求字符串居然有顺序要求。。。
    const queryTemplate = this.basicTemplate(
      'query',
      `db_name=${db}&format=json&sparql=${sparql}`
    );

    return await request({
      uri: queryTemplate,
      method: 'GET',
      json: true
    });
  }

  // make a query url
  makeQuery(db = '', sparql = '') {
    return `${this.basicTemplate(
      'query'
    )}&db_name=${db}&format=json&sparql=${sparql}`;
  }

  async build(db = '', dataPath = '') {
    const buildTemplate = this.basicTemplate(
      'build',
      `&db_name=${db}&ds_path=${dataPath}`
    );

    return await request({
      uri: buildTemplate,
      method: 'GET'
    });
  }

  async load(db = '') {
    const loadTemplate = this.basicTemplate('load', `db_name=${db}`);

    return await request({
      uri: loadTemplate,
      method: 'GET'
    });
  }

  async unload(db = '') {
    const unloadTemplate = this.basicTemplate('unload', `db_name=${db}`);

    return await request({
      uri: unloadTemplate,
      method: 'GET'
    });
  }

  async monitor(db = '') {
    const monitorTemplate = this.basicTemplate('monitor', `db_name=${db}`);

    return await request({
      uri: monitorTemplate,
      method: 'GET'
    });
  }

  async show() {
    const showTemplate = `${this.basicTemplate('show')}`;

    return await request({
      uri: showTemplate,
      method: 'GET'
    });
  }

  async checkpoint() {
    const checkpointTemplate = `${this.basicTemplate('checkpoint')}`;

    return await request({
      uri: checkpointTemplate,
      method: 'GET'
    });
  }
}

module.exports = GStoreClient;
