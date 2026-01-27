import {
  Entity,
  Column,
  PrimaryGeneratedColumn,
  CreateDateColumn,
  UpdateDateColumn,
  DeleteDateColumn,
  OneToOne,
  JoinColumn,
  Index,
} from 'typeorm';
import { User } from '../../users/entities/user.entity';

export enum StudentStatus {
  ACTIVE = 'active',
  INACTIVE = 'inactive',
  GRADUATED = 'graduated',
  SUSPENDED = 'suspended',
}

export enum AcademicLevel {
  LICENCE_1 = 'licence_1',
  LICENCE_2 = 'licence_2',
  LICENCE_3 = 'licence_3',
  MASTER_1 = 'master_1',
  MASTER_2 = 'master_2',
  DOCTORATE = 'doctorate',
}

@Entity('students')
@Index(['userId'], { unique: true })
export class Student {
  @PrimaryGeneratedColumn('uuid')
  id: string;

  @CreateDateColumn()
  createdAt: Date;

  @UpdateDateColumn()
  updatedAt: Date;

  @DeleteDateColumn()
  deletedAt?: Date;

  @Column({ name: 'user_id' })
  userId: string;

  @OneToOne(() => User)
  @JoinColumn({ name: 'user_id' })
  user: User;

  @Column({ name: 'student_number', unique: true, length: 20 })
  studentNumber: string;

  @Column({
    type: 'enum',
    enum: AcademicLevel,
    default: AcademicLevel.LICENCE_1,
  })
  academicLevel: AcademicLevel;

  @Column({
    type: 'enum',
    enum: StudentStatus,
    default: StudentStatus.ACTIVE,
  })
  status: StudentStatus;

  @Column({ name: 'enrollment_date' })
  enrollmentDate: Date;

  @Column({ name: 'specialization', nullable: true, length: 100 })
  specialization?: string;

  @Column({
    name: 'gpa',
    type: 'decimal',
    precision: 3,
    scale: 2,
    nullable: true,
  })
  gpa?: number;

  @Column({ name: 'total_credits', type: 'int', default: 0 })
  totalCredits: number;

  @Column({ name: 'emergency_contact_name', nullable: true, length: 100 })
  emergencyContactName?: string;

  @Column({ name: 'emergency_contact_phone', nullable: true, length: 20 })
  emergencyContactPhone?: string;
}
