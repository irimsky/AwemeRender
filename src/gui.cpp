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
		ImGui::SliderFloat("Scale", &scene.objectScale, 0.01, 30.0);
		ImGui::SliderFloat("Yaw", &scene.objectYaw, -180.0, 180.0);
		ImGui::SliderFloat("Pitch", &scene.objectPitch, -180.0, 180.0);



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
						m_models.push_back(Model(file, false));
						readModelMsg = "";
					}
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
				if (ImGui::Selectable(m_models[i].name.c_str(), isSelected))
				{
					m_selectedIdx = i;
				}
				if (isSelected) {
					ImGui::SetItemDefaultFocus();
					m_models[i].isSelected = true;
				}
				else
				{
					m_models[i].isSelected = false;
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
			ImGui::Text(m_models[i].name.c_str());
			ImGui::DragFloat3(
				"position", m_models[i].position.toPtr(), 0.01f
			);
			ImGui::SliderFloat("scale", &m_models[i].scale, 0.1f, 8.0f);
			ImGui::ColorEdit3("color", m_models[i].color.toPtr());
			ImGui::Separator();

			// 纹理设置
			ImGui::Text(readTextureMsg.c_str());
			int frame_padding = 1;                             
			ImVec2 size = ImVec2(50, 50);
			for (int j = 0; j < Model::TexCount; ++j)
			{
				if (ImGui::ImageButton(
					m_models[i].haveTexture((TextureType)j)
					? (GLuint*)m_models[i].textures[j].id : 0,
					size
				))
				{
					std::string file = File::openFileDialog();
					if (file != "")
					{
						try {
							m_models[i].loadTexture(file, (TextureType)j);
							readTextureMsg = "";
						}
						catch (std::exception e)
						{
							readTextureMsg = "read texture failed";
						}
						
					}
				}
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
