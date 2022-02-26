#include "opengl.hpp"

static std::string readModelMsg = "";
static std::string readTextureMsg = "";
static std::vector<std::string> texNames = {"Albedo Texture"};

void Renderer::renderImgui(SceneSettings& scene)
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	{
		ImGui::Begin("Imgui");
		ImGui::Text("Press [left ALT] to show mouse and control GUI");
		ImGui::Text("FPS:%.2f", scene.FPS);

		static int isForward = 1;
		ImGui::Text("Rendering:"); ImGui::SameLine();
		ImGui::RadioButton("Forward", &isForward, 1); ImGui::SameLine();
		ImGui::RadioButton("Deferred", &isForward, 0);
		scene.isDeferred = !isForward;

		//ImGui::DragFloat("minn", &scene.minn, 0.0001, 0.0001, 0.005, "%.4f");
		//ImGui::DragFloat("maxx", &scene.maxx, 0.0001, 0.0001, 0.1, "%.4f");
		ImGui::SliderFloat("Yaw", &scene.objectYaw, -180.0, 180.0);
		ImGui::SliderFloat("Pitch", &scene.objectPitch, -180.0, 180.0);

		ImGui::Checkbox("skybox", &scene.skybox);
		if (scene.skybox) {
			// 场景切换ComboBox
			if (ImGui::BeginCombo("Scene", scene.envName)) {
				for (int i = 0; i < scene.envNames.size(); i++)
				{
					const bool isSelected = (scene.envName == scene.envNames[i]);
					if (ImGui::Selectable(scene.envNames[i], isSelected)) {
						scene.envName = scene.envNames[i];
						if (strcmp(scene.preEnv, scene.envName))
						{
							loadSceneHdr(scene.envName);
							strcpy(scene.preEnv, scene.envName);
						}
					}
					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
		}
		else
		{
			ImGui::ColorEdit3("ambient light", scene.backgroundColor.toPtr());
		}
		

		ImGui::End();
	}

	// 灯光
	{
		ImGui::Begin("Light");
		// 场景灯光设置
		for (int i = 0; i < scene.NumLights; ++i)
		{
			std::string lightNum = "Light";
			lightNum += '0' + char(i + 1);
			ImGui::Checkbox(lightNum.c_str(), &scene.dirLights[i].enabled);
			if (scene.dirLights[i].enabled) {
				ImGui::ColorEdit3((lightNum + " Color").c_str(), scene.dirLights[i].radiance.toPtr());
				ImGui::DragFloat3((lightNum + " Dir").c_str(), scene.dirLights[i].direction.toPtr(), 0.01f);
			}
		}

		for (int i = 0; i < scene.NumLights; ++i)
		{
			std::string lightNum = "PointLight";
			lightNum += '0' + char(i + 1);
			ImGui::Checkbox(lightNum.c_str(), &scene.ptLights[i].enabled);
			if (scene.ptLights[i].enabled) {
				ImGui::ColorEdit3((lightNum + " Color").c_str(), scene.ptLights[i].radiance.toPtr());
				ImGui::DragFloat3((lightNum + " Pos").c_str(), scene.ptLights[i].position.toPtr(), 1.0f);
			}
		}
		ImGui::End();
	}

	// 场景
	{
		ImGui::Begin("Scene");
		if (ImGui::Button("Add Entity..."))
			ImGui::OpenPopup("add_entity");

		ImGui::SameLine();
		ImGui::Text(readModelMsg.c_str());

		if (ImGui::BeginPopup("add_entity"))
		{
			//ImGui::ShowDemoWindow();
			
			// 从文件中读取模型
			if (ImGui::Selectable("From File..."))
			{
				try {
					std::string file = File::openFileDialog();
					if (file != "") {
						m_models.push_back(ModelPtr((new Model(file, false))));
						readModelMsg = "";
					}
				}
				catch (std::exception e) {
					readModelMsg = "read model failed";
				}
			}
			// 球体
			if (ImGui::Selectable("Sphere"))
			{
				try {
					m_models.push_back(ModelPtr(Model::createSphere()));
					readModelMsg = "";
				}
				catch (std::exception e) {
					readModelMsg = "read model failed";
				}
			}
			// 立方体
			if (ImGui::Selectable("Cube"))
			{
				try {
					m_models.push_back(ModelPtr(Model::createCube()));
					readModelMsg = "";
				}
				catch (std::exception e) {
					readModelMsg = "read model failed";
				}
			}
			// 平面
			if (ImGui::Selectable("Plane"))
			{
				try {
					m_models.push_back(ModelPtr(Model::createPlane()));
					readModelMsg = "";
				}
				catch (std::exception e) {
					readModelMsg = "read model failed";
				}
			}
			
			ImGui::EndPopup();
		}


		if (ImGui::ListBoxHeader("##empty", ImVec2::ImVec2(-FLT_MIN, 0))) {
			for (int i = 0; i < m_models.size(); ++i)
			{
				const bool isSelected = (i == m_selectedIdx);
				if (ImGui::Selectable(m_models[i]->name, isSelected))
				{
					m_selectedIdx = i;
				}
				if (isSelected) {
					ImGui::SetItemDefaultFocus();
					m_models[i]->isSelected = true;
				}
				else
				{
					m_models[i]->isSelected = false;
				}
			}
			ImGui::ListBoxFooter();
		}



		ImGui::End();
	}

	// Entity Info
	{
		ImGui::Begin("Entity");
		if (m_selectedIdx >= 0)
		{
			int i = m_selectedIdx;
			ImGui::InputText("Name", m_models[i]->name, 30);
			ImGui::Text(m_models[i]->name);
			ImGui::DragFloat3(
				"position", m_models[i]->position.toPtr(), 0.01f
			);
			ImGui::DragFloat3(
				"rotate", m_models[i]->rotation.toPtr(), OrbitSpeed
			);
			ImGui::SliderFloat("scale", &m_models[i]->scale, 0.1f, 8.0f);
			ImGui::ColorEdit3("color", m_models[i]->color.toPtr());
			ImGui::Separator();

			// 纹理设置
			ImGui::Text(readTextureMsg.c_str());
			int frame_padding = 1;                             
			ImVec2 size = ImVec2(100, 100);
			for (int j = 0; j < Model::TexCount; ++j)
			{
				ImGui::PushID(j);
				if (ImGui::ImageButton(
					m_models[i]->haveTexture((TextureType)j)
					? (GLuint*)m_models[i]->textures[j].id : 0,
					//(GLuint*)m_interFramebuffer.depthStencilTarget,
					size
				))
				{
					std::string file = File::openFileDialog();
					if (file != "")
					{
						try {
							m_models[i]->loadTexture(file, (TextureType)j);
							readTextureMsg = "";
						}
						catch (std::exception e)
						{
							readTextureMsg = "read texture failed";
						}
						
					}
				}
				ImGui::PopID();
				ImGui::SameLine();
				auto lineHeight = 20;
				std::string tmp = TextureTypeNames[j];
				auto textHeight = ImGui::CalcTextSize(tmp.c_str()).y;
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() +
					(lineHeight - textHeight) * 1.5f);
				tmp += " Texture";
				ImGui::Text(tmp.c_str());
			}
		}
		ImGui::End();
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
