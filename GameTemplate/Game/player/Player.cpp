#include "stdafx.h"
#include "Player.h"

namespace nsMyGame {

	namespace nsPlayer {

		CPlayer* g_pCurrentPlayer = nullptr;

		void Normal() {
			g_pCurrentPlayer->GetModelRender()->PlayAnimation(enAnim_Idle, c_animationInterpolateTime);
		}

		void Yoi() {
			g_pCurrentPlayer->GetModelRender()->PlayAnimation(enAnim_Walk, c_animationInterpolateTime);
		}

		void Death() {
			g_pCurrentPlayer->GetModelRender()->PlayAnimation(enAnim_Death, c_animationInterpolateTime);
		}

		int GetYoiParam() {

			return g_pCurrentPlayer->GetYoiParam();
		}

		void ChangeState(int state) {

			g_pCurrentPlayer->SetState(state);
		}

		void Move() {
			g_pCurrentPlayer->Move();
		}

		void Drink() {

			if (g_pad[0]->IsTrigger(enButtonA)) {
				int yoiParam = g_pCurrentPlayer->GetYoiParam();
				yoiParam += 10;
				g_pCurrentPlayer->SetYoiParam(yoiParam);
			}
		}

		void LightCameraUpdate() {
			g_pCurrentPlayer->LightCameraUpdate();
		}

		void FontUpdate() {
			g_pCurrentPlayer->FontUpdate();
		}

		PYBIND11_MODULE(Game, m) {
			m.def("Normal", &Normal);
			m.def("Yoi", &Yoi);
			m.def("Death", &Death);
			m.def("Move", &Move);
			m.def("Drink", &Drink);
			m.def("GetYoiParam", &GetYoiParam);
			m.def("ChangeState", &ChangeState);
			m.def("LightCameraUpdate", &LightCameraUpdate);
			m.def("FontUpdate", &FontUpdate);
		}






		



		bool CPlayer::Start() {

			m_modelRender = NewGO<CModelRender>(0);
			m_modelRender->SetFilePathTkm("Assets/modelData/knight.tkm");
			m_modelRender->SetFilePathTks("Assets/modelData/knight.tks");
			m_modelRender->SetFilePathTkm("Assets/unityChanBeer.tkm");
			m_modelRender->SetFilePathTks("Assets/unityChanBeer.tks");
			m_animationClip[enAnim_Walk].Load("Assets/animData/walk.tka");
			m_animationClip[enAnim_Idle].Load("Assets/animData/Idle.tka");
			m_animationClip[enAnim_Death].Load("Assets/animData/KneelDown.tka");
			m_animationClip[enAnim_Walk].SetLoopFlag(true);
			m_animationClip[enAnim_Idle].SetLoopFlag(true);
			m_animationClip[enAnim_Death].SetLoopFlag(false);
			m_modelRender->InitAnimation(m_animationClip, enAnimNum);
			m_modelRender->SetAnimFlg(true);
			m_modelRender->SetShadowCasterFlag(true);
			m_modelRender->SetShadowReceiverFlag(true);
			m_modelRender->SetModelUpAxis(EnModelUpAxis::enModelUpAxisY);
			m_modelRender->Init();

			m_fontRender = NewGO<nsFont::CFontRender>(0);
			m_fontRender->Init(L"�����x�F", { -550.0f,310.0f });	//�ꏊ
			m_fontRender->SetColor({ 1.0f,0.0f,0.0f,1.0f });			//�ԐF
			m_fontRender->SetShadowParam(true, 1.0f, CVector4::Black);

			m_fontRender2 = NewGO<nsFont::CFontRender>(0);
			m_fontRender2->Init(L"A�F����", { 450.0f,310.0f });	//�ꏊ
			m_fontRender2->SetColor({ 1.0f,0.0f,0.0f,1.0f });			//�ԐF
			m_fontRender2->SetShadowParam(true, 1.0f, CVector4::Black);


			//�L�����N�^�[�R���g���[���[���������B
			m_charaCon.Init(
				20.0f,			//���a�B
				100.0f,			//�����B
				m_position		//���W�B
			);
			return true;
		}

		void CPlayer::StateUpdate() {

			//���݂̃C���X�^���X�����B
			g_pCurrentPlayer = this;

			//Python�X�N���v�g�����[�h����B
			switch (m_playerState) {
			case EnPlayerState::enState_Normal:

				//�ʏ�
				ImportModule("PlayerNormal");
				break;
			case EnPlayerState::enState_Yoi:

				//�ق�悢
				ImportModule("PlayerYoi");
				break;
			case EnPlayerState::enState_Death:

				//�}���A���R�[������
				ImportModule("PlayerDeath");
				break;
			}

			//Python�X�N���v�g�̒���Update()�֐����Ăяo���B
			auto updateFunc = m_playerPyModule.attr("Update");
			updateFunc();
		}


		void CPlayer::Update() {

			//�X�e�[�g�ɂ���ēǂݍ���Python�X�N���v�g��؂�ւ���B
			StateUpdate();
		}












		void CPlayer::LightCameraUpdate() {

			//���C�g�J�����̏����X�V
			CVector3 m_lightCameraTar = m_position;
			CVector3 m_lightCameraPos = m_lightCameraTar;

			//���C�g�J�����̍��W��ݒ�
			m_lightCameraPos.y += 300.0f;
			m_lightCameraPos.z += 300.0f;

			//���C�g�J�����̏���ݒ�
			CCamera::GetLightCamera()->SetPosition(m_lightCameraPos);
			CCamera::GetLightCamera()->SetTarget(m_lightCameraTar);
			CCamera::GetLightCamera()->SetFar(800.0f);

			//�J�����̏���������߂�B
			CVector3 Vec_x = CVector3::AxisX;
			CVector3 TarPos = CCamera::GetLightCamera()->GetTarget() - CCamera::GetLightCamera()->GetPosition();
			TarPos.Normalize();
			CVector3 CameraUp;

			CameraUp.Cross(TarPos, Vec_x);
			CameraUp.Normalize();
			CCamera::GetLightCamera()->SetUp(CameraUp);
			CCamera::GetLightCamera()->Update();
		}

		void CPlayer::Move() {

			m_moveSpeed.x = 0.0f;
			m_moveSpeed.z = 0.0f;
			
			//���̃t���[���̈ړ��ʂ����߂�B
			//���X�e�B�b�N�̓��͗ʂ��󂯎��B
			float lStick_x = g_pad[0]->GetLStickXF();
			float lStick_y = g_pad[0]->GetLStickYF();
			//�J�����̑O�������ƉE�������擾�B
			CVector3 cameraForward = g_camera3D->GetForward();
			CVector3 cameraRight = g_camera3D->GetRight();
			//XZ���ʂł̑O�������A�E�����ɕϊ�����B
			cameraForward.y = 0.0f;
			cameraForward.Normalize();
			cameraRight.y = 0.0f;
			cameraRight.Normalize();
			//XZ�����̈ړ����x���N���A�B
			m_moveSpeed += cameraForward * lStick_y * 500.0f;	//�������ւ̈ړ����x�����Z�B
			m_moveSpeed += cameraRight * lStick_x * 500.0f;		//�E�����ւ̈ړ����x�����Z�B
			if (g_pad[0]->IsTrigger(enButtonA) //A�{�^���������ꂽ��
				//&& m_charaCon.IsOnGround()  //���A�n�ʂɋ�����
				) {
				//�W�����v����B
				m_moveSpeed.y = 400.0f;	//������ɑ��x��ݒ肵�āA
			}

			m_moveSpeed.y -= 980.0f * g_gameTime->GetFrameDeltaTime();
			//�L�����N�^�[�R���g���[���[���g�p���āA���W���X�V�B

			m_position = m_charaCon.Execute(m_moveSpeed, g_gameTime->GetFrameDeltaTime());
			if (m_charaCon.IsOnGround()) {
				//�n�ʂɂ����B
				m_moveSpeed.y = 0.0f;
			}

			//���W��ݒ�B
			m_modelRender->SetPosition(m_position);
		}

		void CPlayer::FontUpdate() {

			wchar_t time[64];
			swprintf_s(time, L"�����x�F%d", m_yoiParam);

			if (m_yoiParam >= 60 && m_yoiParam < 100) {
				m_color = { 1.0f,0.0f,0.0f,1.0f };
			}
			else if (m_yoiParam >= 100) {
				switch (m_colorState) {
				case en1:
					m_color.y += 0.05f;
					if (m_count == 20) {
						m_colorState = en2;
						m_count = 0;
					}
					break;
				case en2:
					m_color.x -= 0.05f;
					if (m_count == 20) {
						m_colorState = en3;
						m_count = 0;
					}
					break;
				case en3:
					m_color.z += 0.05f;
					if (m_count == 20) {
						m_colorState = en4;
						m_count = 0;
					}
					break;
				case en4:
					m_color.y -= 0.05f;
					if (m_count == 20) {
						m_colorState = en5;
						m_count = 0;
					}
					break;
				case en5:
					m_color.x += 0.05f;
					if (m_count == 20) {
						m_colorState = en6;
						m_count = 0;
					}
					break;
				case en6:
					m_color.z -= 0.05f;
					if (m_count == 20) {
						m_colorState = en1;
						m_count = 0;
					}
					break;
				}
				m_count++;
				swprintf_s(time, L"�����x�F�}���A���R�[������");
			}
			m_fontRender->SetText(time);
			m_fontRender->SetColor(m_color);
		}
	}
}