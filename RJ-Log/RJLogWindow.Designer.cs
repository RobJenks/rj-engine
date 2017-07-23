namespace RJ_Log
{
    partial class RJLogWindow
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            this.LogData = new System.Windows.Forms.RichTextBox();
            this.updateTimer = new System.Windows.Forms.Timer(this.components);
            this.SuspendLayout();
            // 
            // LogData
            // 
            this.LogData.BackColor = System.Drawing.Color.Black;
            this.LogData.Dock = System.Windows.Forms.DockStyle.Fill;
            this.LogData.Font = new System.Drawing.Font("Consolas", 10F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.LogData.ForeColor = System.Drawing.Color.Gainsboro;
            this.LogData.Location = new System.Drawing.Point(0, 0);
            this.LogData.Name = "LogData";
            this.LogData.ReadOnly = true;
            this.LogData.Size = new System.Drawing.Size(1008, 453);
            this.LogData.TabIndex = 0;
            this.LogData.Text = "";
            // 
            // updateTimer
            // 
            this.updateTimer.Interval = 500;
            this.updateTimer.Tick += new System.EventHandler(this.updateTimer_Tick);
            // 
            // RJLogWindow
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.SystemColors.ActiveBorder;
            this.ClientSize = new System.Drawing.Size(1008, 453);
            this.Controls.Add(this.LogData);
            this.Name = "RJLogWindow";
            this.Text = "RJ-Log";
            this.Load += new System.EventHandler(this.RJLogWindow_Load);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.RichTextBox LogData;
        private System.Windows.Forms.Timer updateTimer;
    }
}

