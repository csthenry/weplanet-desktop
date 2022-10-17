<template>
  <div class="dashboard-container">
    <div class="app-container">
      <!-- 放置内容 -->
      <el-card>
        <el-tabs>
          <el-tab-pane label="人员管理">
            <!-- 人员管理内容 -->
            <el-row style="height:60px">
              <el-button icon="el-icon-plus" type="primary" size="small">新增人员</el-button>
            </el-row>
            <el-table
              :data="grouplist"
              border
              style="width: 100%"
            >
              <el-table-column
                align="center"
                label="序号"
                width="120"
                type="index"
              />
              <el-table-column
                prop="name"
                label="人员名称"
                width="240"
                align="center"
              />
              <el-table-column
                prop="description"
                label="描述"
                align="center"
              />
              <el-table-column
                prop="companyId"
                label="操作"
                align="center"
              >
                <!-- 作用域插槽 -->
                <template slot-scope="{ row }">
                  <el-button size="small" type="success">分配权限</el-button>
                  <el-button size="small" type="primary" @click="editRole(row.id)">编辑</el-button>
                  <el-button size="small" type="danger" @click="deletRole(row.id)">删除</el-button>
                </template>

              </el-table-column>
            </el-table>
            <!-- 插入分页组件 -->
            <el-row type="flex" justify="center" align="middle" style="height:60px">
              <!-- 分页组件 -->
              <el-pagination
                :current-page="page.page"
                :page-size="page.pagesize"
                :total="page.total"
                layout="prev,pager,next"
                @current-change="changePage"
              />
            </el-row>
          </el-tab-pane>
          <el-tab-pane label="公司信息" name="second">
            <el-alert
              title="对公司名称、公司地址、营业执照、公司地区的更新，将使得公司资料被重新审核，请谨慎修改"
              type="info"
              show-icon
              :closable="false"
            />
            <!-- 右侧公司设置 -->
            <!-- 在做表单校验时并不是所有的表单都需要 model rules 来执行校验-->
            <el-form label-width="120px" style="margin-top:50px">
              <el-form-item label="公司名称">
                <el-input v-model="formData.name" disabled style="width:400px" />
              </el-form-item>
              <el-form-item label="公司地址">
                <el-input v-model="formData.companyAddress" disabled style="width:400px" />
              </el-form-item>
              <el-form-item label="公司电话">
                <el-input v-model="formData.companyPhone" disabled style="width:400px" />
              </el-form-item>
              <el-form-item label="邮箱">
                <el-input v-model="formData.mailbox" disabled style="width:400px" />
              </el-form-item>
              <el-form-item label="备注">
                <el-input v-model="formData.remarks" type="textarea" :rows="3" disabled style="width:400px" />
              </el-form-item>
            </el-form>
          </el-tab-pane>
        </el-tabs>
      </el-card>
    </div>
    <!-- 放置弹层组件 -->
    <el-dialog title="编辑部门" :visible="showDialog">
      <el-form
        ref="roleForm"
        label-width="120px"
        :model="roleForm"
        :rules="rules"
      >
        <el-form-item label="角色名称" prop="name">
          <el-input v-model="roleForm.description" />
        </el-form-item>
        <el-form-item label="角色描述">
          <el-input />
        </el-form-item>
      </el-form>
      <!-- 底部 -->
      <el-row slot="footer" type="flex" justify="center">
        <el-col :span="6">
          <el-button size="small" type="primary">确定</el-button>
          <el-button size="small">取消</el-button>
        </el-col>
      </el-row>
    </el-dialog>
  </div>
</template>

<script>
import { getRoleList, getCompanyInfo, deleteRole } from '@/api/setting'
import { mapGetters } from 'vuex'
export default {
  data() {
    return {
      // 人员列表
      grouplist: [], // 承接数组里的数据
      page: {
        // 放置页码及相关数据
        page: 1, // 默认页码
        pagesize: 10, // 每页条数
        total: 0 // 记录总数 默认为0
      },
      formData: {
        // 公司信息
      },
      showDialog: false, // 控制弹层显示
      // // 专门接收新增或者编辑的编辑的表单数据
      roleForm: {
        name: '',
        description: ''
      },
      rules: {
        name: [{ required: true, message: '角色名称不能为空', trigger: 'blur' }]
      }
    }
  },
  computed: {
    ...mapGetters(['companyId'])
  },
  created() {
    this.getRoleList() // 获取人员列表数据
    this.getCompanyInfo() // 获取公司信息
  },
  methods: {
    // 调用接口 并且解构出需要的数据
    async getRoleList() {
      const { total, rows } = await getRoleList(this.page)
      this.page.total = total
      this.grouplist = rows // rows是每一行的数据
    },
    async getCompanyInfo() {
      this.formData = await getCompanyInfo(this.companyId)
    },
    changePage(newPage) {
      // newPage 参数时当前点击的页码
      this.page.page = newPage// 将当前点击页码赋值给默认页码
      this.getRoleList() // 每次点击获取最新数据
    },
    async deletRole(id) {
      // 提示是否删除
      try {
        await this.$confirm('确认删除该人员信息吗')
        // 只有点击确定 才能执行下方代码

        await deleteRole(id) // 调用删除接口
        this.getRoleList() // 重新加载数据
        this.$message.success('删除人员成功') // 提示成功信息
      } catch (error) {
        console.log(error)
      }
    },
    editRole(id) {
      this.showDialog = true // 点击 显示弹层
    }
  }

}
</script>

<style>

</style>
