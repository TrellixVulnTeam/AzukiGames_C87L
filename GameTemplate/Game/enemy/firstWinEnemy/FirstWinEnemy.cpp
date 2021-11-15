#include "stdafx.h"
#include "FirstWinEnemy.h"

namespace nsMyGame {

	namespace nsEnemy {

		extern CEnemy* g_pCurrentEnemy;

		bool CFirstWinEnemy::StartSub() {
		
			//初期座標を設定。
			//m_position = { 1500.0f,500.0f,-1200.0f };

			//IGameObjectに追加。
			m_modelRender = NewGO<CModelRender>(enPriority_Zeroth);
		
			//tkmファイルとtksファイルを設定。
			m_modelRender->SetFilePathTkm(c_filePathTkmEnemy);
			m_modelRender->SetFilePathTks(c_filePathTksEnemy);

			//座標を設定。
			m_modelRender->SetPosition(m_position);

			//回転を設定。
			m_modelRender->SetRotation(m_rotation);
		
			//アニメーションクリップを初期化。
			InitAnimationClip();
		
			//アニメーションを初期化。
			m_modelRender->InitAnimation(m_animationClip, enAnim_Num);
		
			//影を落とす。
			m_modelRender->SetShadowCasterFlag(true);
		
			//影を受ける。
			m_modelRender->SetShadowReceiverFlag(true);
		
			//ステータスを初期化。
			InitStatus();

			//初期化。
			m_modelRender->Init();

			//キャラクターコントローラーを初期化。
			m_charaCon.Init(
				20.0f,			//半径。
				100.0f,			//高さ。
				m_position		//座標。
			);
			return true;
		}

		void CFirstWinEnemy::UpdateSub() {

			// 現在更新処理を実行中のエネミーのアドレスを代入
			g_pCurrentEnemy = this;
			
			//ステートに応じて読み込むPythonスクリプトを変える。
			switch (m_state) {
			case enState_Idle:
				ImportModule("EnemyIdle");
				break;
			case enState_Walk:
				ImportModule("EnemyMove");
				break;
			case enState_ThreeCombo:
				ImportModule("EnemyAttack");
				break;
			case enState_Damage:
				ImportModule("EnemyDamage");
				break;
			case enState_Death:
				ImportModule("EnemyDeath");
				break;
			case enState_AttackBreak:
				ImportModule("EnemyAttackBreak");
				break;
			}

			//PythonスクリプトのUpdate()関数を呼び出す。
			auto updateFunc = m_enemyPyModule.attr("Update");
			updateFunc();
		}

		void CFirstWinEnemy::InitStatus() {

			m_status.hp = 100;
			m_status.attack = 10;
		}

		void CFirstWinEnemy::InitAnimationClip() {

			//アニメーションクリップを設定。
			m_animationClip[enAnim_Walk].Load("Assets/animData/walk.tka");
			m_animationClip[enAnim_Walk].SetLoopFlag(true);
			m_animationClip[enAnim_Idle].Load("Assets/animData/idle.tka");
			m_animationClip[enAnim_Idle].SetLoopFlag(true);
			m_animationClip[enAnim_ThreeCombo].Load("Assets/animData/threeCombo.tka");
			m_animationClip[enAnim_ThreeCombo].SetLoopFlag(false);
			m_animationClip[enAnim_Damage].Load("Assets/animData/damage.tka");
			m_animationClip[enAnim_Damage].SetLoopFlag(false);
			m_animationClip[enAnim_Death].Load("Assets/animData/death.tka");
			m_animationClip[enAnim_Death].SetLoopFlag(false);
			m_animationClip[enAnim_AttackBreak].Load("Assets/animData/attackBreak.tka");
			m_animationClip[enAnim_AttackBreak].SetLoopFlag(false);
		}

		void CFirstWinEnemy::AnimationUpdate() {

			//各ステートに対応したアニメーションを再生する。
			switch (m_state) {
			case enState_Idle:
				m_modelRender->PlayAnimation(enAnim_Idle, 0.4f);
				break;
			case enState_Walk:
				m_modelRender->PlayAnimation(enAnim_Walk, 0.4f);
				break;
			case enState_ThreeCombo:
				m_modelRender->PlayAnimation(enAnim_ThreeCombo, 0.4f);
				break;
			case enState_Damage:
				m_modelRender->PlayAnimation(enAnim_Damage, 0.4f);
				break;
			case enState_Death:
				m_modelRender->PlayAnimation(enAnim_Death, 0.4f);
				break;
			case enState_AttackBreak:
				m_modelRender->PlayAnimation(enAnim_AttackBreak, 0.4f);
				break;
			}
		}

		void CFirstWinEnemy::Move() {

			//x方向とz方向の移動速度を初期化。
			m_moveSpeed.x = 0.0f;
			m_moveSpeed.z = 0.0f;

			//プレイヤーの座標を取得する。
			CVector3 toPlayerVec = m_player->GetPosition() - m_position;
			//正規化。
			toPlayerVec.Normalize();

			//歩き状態ならプレイヤーに一定速度で近づく。
			if (m_state == enState_Walk) {

				m_moveSpeed += toPlayerVec * 150.0f;
			}

			//3連続攻撃状態なら一定速度でプレイヤーに近づく。
			else if (m_state == enState_ThreeCombo) {

				if (c_threeComboCoolTime - m_coolTime < 2.4f) {
					m_moveSpeed += toPlayerVec * 50.0f;
				}
			}

			//重力をかける。
			m_moveSpeed.y -= 980.0f * g_gameTime->GetFrameDeltaTime();

			//座標を設定。
			m_position = m_charaCon.Execute(m_moveSpeed, g_gameTime->GetFrameDeltaTime());

			//地面についているか判定。
			if (m_charaCon.IsOnGround()) {

				//地面についているなら下向きには力をかけない。
				m_moveSpeed.y = 0.0f;
			}
		}

		void CFirstWinEnemy::UpdateTriggerBox(const CVector3& pos, const CQuaternion& rot, const CVector3& forward) {

			//3連続攻撃状態なら
			if (m_state == enState_ThreeCombo) {

				//斬るタイミングでトリガーボックスを有効にする。
				if (m_coolTime > 0.4f && m_coolTime < 0.8f) {

					m_triggerBox.Activate(pos, rot);
				}
				else if (m_coolTime > 1.3f && m_coolTime < 1.7f) {

					m_triggerBox.Activate(pos, rot);
				}
				else if (m_coolTime > 2.0f && m_coolTime < 2.4f) {

					m_triggerBox.Activate(pos, rot);
				}
				//それ以外は無効にする。
				else {
					m_triggerBox.Deactivate();
				}
			}
			//攻撃時以外は無効にする。
			else {

				m_triggerBox.Deactivate();
			}

			//トリガーボックスを更新。
			m_triggerBox.Update(pos, rot, forward);
		}
	}
}